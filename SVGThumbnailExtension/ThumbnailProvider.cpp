/*
 * SVG Preview - SVG thumbnails for Windows Explorer.
 * Copyright (C) 2026 Gabriel Sartori.
 * Copyright (C) 2012-2020 Tibold Kandrai and the SVG Explorer Extension contributors.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version. See LICENSE.md for the full text.
 */
#include "Common.h"
#include "ThumbnailProvider.h"
#include "Logging.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <mutex>
#include <vector>

namespace {

// Shared, lazily created resvg options. Loading the system font database is
// expensive, so it is done once per process and reused by every instance.
// resvg_parse_tree_from_data only borrows the options (const), so sharing a
// single instance across apartment threads is safe.
const resvg_options* sharedOptions()
{
    static resvg_options* options = nullptr;
    static std::once_flag once;
    std::call_once(once, []() {
        options = resvg_options_create();
        resvg_options_load_system_fonts(options);
    });
    return options;
}

} // namespace

CThumbnailProvider::CThumbnailProvider()
{
    DllAddRef();
    m_cRef = 1;
    m_pSite = NULL;
    m_tree = NULL;
}

CThumbnailProvider::~CThumbnailProvider()
{
    if (m_tree)
    {
        resvg_tree_destroy(m_tree);
        m_tree = NULL;
    }
    if (m_pSite)
    {
        m_pSite->Release();
        m_pSite = NULL;
    }
    DllRelease();
}

/*
 * ===============
 * IUnkown methods
 * ===============
 */
HRESULT CThumbnailProvider::QueryInterfaceFactory(REFIID riid, void** ppvObject)
{
    *ppvObject = NULL;

    CThumbnailProvider * provider = new CThumbnailProvider();
    if (provider == nullptr)
    {
        return E_OUTOFMEMORY;
    }

    auto result = provider->QueryInterface(riid, ppvObject);

    provider->Release();

    return result;
}

STDMETHODIMP CThumbnailProvider::QueryInterface(REFIID riid,
                                                void** ppvObject)
{
    static const QITAB qit[] =
    {
        QITABENT(CThumbnailProvider, IInitializeWithStream),
        QITABENT(CThumbnailProvider, IThumbnailProvider),
        QITABENT(CThumbnailProvider, IObjectWithSite),
        {0},
    };
    return QISearch(this, qit, riid, ppvObject);
}

STDMETHODIMP_(ULONG) CThumbnailProvider::AddRef()
{
    LONG cRef = InterlockedIncrement(&m_cRef);
    return (ULONG)cRef;
}

STDMETHODIMP_(ULONG) CThumbnailProvider::Release()
{
    LONG cRef = InterlockedDecrement(&m_cRef);
    if (0 == cRef)
        delete this;
    return (ULONG)cRef;
}

/*
 * ===============
 * End IUnkown methods
 * ===============
 */

/*
 * ============================
 * IInitializeWithSteam methods
 * ============================
 */

STDMETHODIMP CThumbnailProvider::Initialize(IStream *pstm,
                                            DWORD grfMode)
{
    STATSTG stat;
    UNREFERENCED_PARAMETER(grfMode);

    if(m_tree) {
        return HRESULT_FROM_WIN32(ERROR_ALREADY_INITIALIZED);
    }

    // STATFLAG_NONAME: the name is not needed and would have to be freed.
    if(pstm->Stat(&stat, STATFLAG_NONAME) != S_OK){
        return E_FAIL;
    }

    // Reject empty streams and refuse absurdly large files instead of
    // attempting an allocation of that size.
    const ULONGLONG maxSvgSize = 256ull * 1024 * 1024;
    if(stat.cbSize.QuadPart == 0 || stat.cbSize.QuadPart > maxSvgSize){
        return E_FAIL;
    }

    std::vector<char> bytes(static_cast<size_t>(stat.cbSize.QuadPart));

    // IStream::Read may return less than requested; keep reading until the
    // whole stream is consumed.
    ULONG total = 0;
    while(total < bytes.size()){
        ULONG read = 0;
        HRESULT hr = pstm->Read(bytes.data() + total,
                                static_cast<ULONG>(bytes.size()) - total,
                                &read);
        if(FAILED(hr) || read == 0){
            break;
        }
        total += read;
        if(hr == S_FALSE){
            break;
        }
    }
    if(total != bytes.size()){
        return E_FAIL;
    }

    // resvg handles both plain SVG and gzip compressed SVGZ data.
    int32_t err = resvg_parse_tree_from_data(bytes.data(),
                                             bytes.size(),
                                             sharedOptions(),
                                             &m_tree);
    if(err != RESVG_OK){
        debugLog << "Failed to parse SVG data, resvg error " << err;
        m_tree = NULL;
        return E_FAIL;
    }

    return S_OK;
}

/*
 * ============================
 * End IInitializeWithSteam methods
 * ============================
 */

/*
 * ============================
 * IThumbnailProvider methods
 * ============================
 */

STDMETHODIMP CThumbnailProvider::GetThumbnail(UINT cx,
                                              HBITMAP *phbmp,
                                              WTS_ALPHATYPE *pdwAlpha)
{
    *phbmp = NULL;
    *pdwAlpha = WTSAT_ARGB;

    if(!m_tree){
        return S_FALSE;
    }

    // Fit the render into a (cx * cx) square while maintaining the aspect ratio.
    resvg_size docSize = resvg_get_image_size(m_tree);
    if(!(docSize.width > 0.0f) || !(docSize.height > 0.0f)){
        return S_FALSE;
    }

    const double scale = (std::min)(cx / (double)docSize.width,
                                    cx / (double)docSize.height);
    const LONG width  = (std::max)(1l, std::lround(docSize.width * scale));
    const LONG height = (std::max)(1l, std::lround(docSize.height * scale));

    // Top-down 32bpp DIB; the thumbnail cache expects premultiplied BGRA.
    BITMAPINFO bmi;
    ZeroMemory(&bmi, sizeof(bmi));
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = width;
    bmi.bmiHeader.biHeight = -height;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    void* bits = NULL;
    HBITMAP bitmap = CreateDIBSection(NULL, &bmi, DIB_RGB_COLORS, &bits, NULL, 0);
    if(bitmap == NULL || bits == NULL){
        if(bitmap){
            DeleteObject(bitmap);
        }
        return E_OUTOFMEMORY;
    }

    const size_t byteCount = (size_t)width * (size_t)height * 4;
    std::memset(bits, 0, byteCount);

    // resvg renders premultiplied RGBA8888; render straight into the DIB and
    // swap R and B in place to obtain the premultiplied BGRA GDI expects.
    resvg_transform transform = resvg_transform_identity();
    transform.a = (float)scale;
    transform.d = (float)scale;
    resvg_render(m_tree, transform,
                 (uint32_t)width, (uint32_t)height,
                 static_cast<char*>(bits));

    unsigned char* px = static_cast<unsigned char*>(bits);
    for(size_t i = 0; i < byteCount; i += 4){
        unsigned char r = px[i];
        px[i] = px[i + 2];
        px[i + 2] = r;
    }

    *phbmp = bitmap;
    return S_OK;
}

/*
 * ============================
 * End IThumbnailProvider methods
 * ============================
 */

/*
 * ============================
 * IObjectWithSite methods
 * ============================
 */

STDMETHODIMP CThumbnailProvider::GetSite(REFIID riid,
                                         void** ppvSite)
{
    if (m_pSite)
    {
        return m_pSite->QueryInterface(riid, ppvSite);
    }
    return E_NOINTERFACE;
}

STDMETHODIMP CThumbnailProvider::SetSite(IUnknown* pUnkSite)
{
    if (m_pSite)
    {
        m_pSite->Release();
        m_pSite = NULL;
    }

    m_pSite = pUnkSite;
    if (m_pSite)
    {
        m_pSite->AddRef();
    }
    return S_OK;
}

/*
 * ============================
 * End IObjectWithSite methods
 * ============================
 */
