// Offline test harness for the SvgPreview thumbnail provider.
//
// Loads the shell extension DLL the same way Explorer does -- via
// DllGetClassObject -> IClassFactory -> IInitializeWithStream ->
// IThumbnailProvider -- and saves the produced HBITMAP as a PNG so the
// output can be inspected without registering the extension.
//
// Usage: ThumbnailTestHarness <SvgPreview.dll> <input.svg> <output.png> [size]

#include <windows.h>
#include <shlwapi.h>
#include <thumbcache.h>
#include <objbase.h>
#include <gdiplus.h>

#include <cstdio>
#include <cwchar>

// Must match szCLSID_SampleThumbnailProvider in Common.h
static const CLSID CLSID_SvgPreviewThumbnailProvider =
    {0x927c8f59, 0xc5ea, 0x488c, {0x99, 0x5e, 0xa6, 0x1a, 0xf4, 0x02, 0xdf, 0x7d}};

typedef HRESULT (STDAPICALLTYPE *DllGetClassObjectFunc)(REFCLSID, REFIID, void**);

static int GetPngEncoderClsid(CLSID* clsid)
{
    UINT count = 0;
    UINT size = 0;
    Gdiplus::GetImageEncodersSize(&count, &size);
    if (size == 0)
        return -1;

    Gdiplus::ImageCodecInfo* info = (Gdiplus::ImageCodecInfo*)malloc(size);
    if (info == NULL)
        return -1;

    Gdiplus::GetImageEncoders(count, size, info);
    for (UINT i = 0; i < count; ++i)
    {
        if (wcscmp(info[i].MimeType, L"image/png") == 0)
        {
            *clsid = info[i].Clsid;
            free(info);
            return (int)i;
        }
    }
    free(info);
    return -1;
}

static int Fail(const wchar_t* step, HRESULT hr)
{
    fwprintf(stderr, L"FAILED %s (hr=0x%08lX)\n", step, (unsigned long)hr);
    return 1;
}

int wmain(int argc, wchar_t** argv)
{
    if (argc < 4)
    {
        fwprintf(stderr, L"Usage: %s <SvgPreview.dll> <input.svg> <output.png> [size]\n", argv[0]);
        return 2;
    }

    const wchar_t* dllPath = argv[1];
    const wchar_t* svgPath = argv[2];
    const wchar_t* pngPath = argv[3];
    UINT size = (argc > 4) ? (UINT)_wtoi(argv[4]) : 256;

    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    if (FAILED(hr))
        return Fail(L"CoInitializeEx", hr);

    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken = 0;
    Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    HMODULE dll = LoadLibraryW(dllPath);
    if (dll == NULL)
    {
        fwprintf(stderr, L"FAILED LoadLibrary(%s), error %lu\n", dllPath, GetLastError());
        return 1;
    }

    DllGetClassObjectFunc getClassObject =
        (DllGetClassObjectFunc)GetProcAddress(dll, "DllGetClassObject");
    if (getClassObject == NULL)
    {
        fwprintf(stderr, L"FAILED GetProcAddress(DllGetClassObject)\n");
        return 1;
    }

    IClassFactory* factory = NULL;
    hr = getClassObject(CLSID_SvgPreviewThumbnailProvider, IID_PPV_ARGS(&factory));
    if (FAILED(hr))
        return Fail(L"DllGetClassObject", hr);

    IInitializeWithStream* init = NULL;
    hr = factory->CreateInstance(NULL, IID_PPV_ARGS(&init));
    factory->Release();
    if (FAILED(hr))
        return Fail(L"CreateInstance(IInitializeWithStream)", hr);

    IStream* stream = NULL;
    hr = SHCreateStreamOnFileEx(svgPath, STGM_READ | STGM_SHARE_DENY_WRITE,
                                FILE_ATTRIBUTE_NORMAL, FALSE, NULL, &stream);
    if (FAILED(hr))
        return Fail(L"SHCreateStreamOnFileEx", hr);

    hr = init->Initialize(stream, STGM_READ);
    stream->Release();
    if (FAILED(hr))
        return Fail(L"IInitializeWithStream::Initialize", hr);

    IThumbnailProvider* provider = NULL;
    hr = init->QueryInterface(IID_PPV_ARGS(&provider));
    init->Release();
    if (FAILED(hr))
        return Fail(L"QueryInterface(IThumbnailProvider)", hr);

    HBITMAP bitmap = NULL;
    WTS_ALPHATYPE alphaType;
    hr = provider->GetThumbnail(size, &bitmap, &alphaType);
    provider->Release();
    if (hr != S_OK || bitmap == NULL)
        return Fail(L"IThumbnailProvider::GetThumbnail", hr);

    DIBSECTION dib;
    ZeroMemory(&dib, sizeof(dib));
    if (GetObject(bitmap, sizeof(dib), &dib) == 0 || dib.dsBm.bmBits == NULL)
        return Fail(L"GetObject(DIBSECTION)", E_FAIL);

    wprintf(L"Thumbnail: %ldx%ld, alpha type %d\n",
            dib.dsBm.bmWidth, dib.dsBm.bmHeight, (int)alphaType);

    {
        // The DIB was created top-down (negative biHeight), so rows are in
        // natural order and a positive stride is correct here.
        Gdiplus::Bitmap out(dib.dsBm.bmWidth, dib.dsBm.bmHeight,
                            dib.dsBm.bmWidthBytes, PixelFormat32bppPARGB,
                            (BYTE*)dib.dsBm.bmBits);
        CLSID pngClsid;
        if (GetPngEncoderClsid(&pngClsid) < 0)
            return Fail(L"GetPngEncoderClsid", E_FAIL);

        if (out.Save(pngPath, &pngClsid, NULL) != Gdiplus::Ok)
            return Fail(L"Bitmap::Save", E_FAIL);
    }

    DeleteObject(bitmap);
    Gdiplus::GdiplusShutdown(gdiplusToken);
    CoUninitialize();

    wprintf(L"OK: %s\n", pngPath);
    return 0;
}
