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
#include "ClassFactory.h"
#include "ThumbnailProvider.h"

CClassFactory::CClassFactory()
{
    m_cRef = 1;
    DllAddRef();
}

CClassFactory::~CClassFactory()
{
    DllRelease();
}

HRESULT CClassFactory::QueryInterfaceFactory(REFIID riid,
                                             void** ppvObject)
{
    CClassFactory * factory = new CClassFactory();
    if (factory == nullptr) {
        return E_OUTOFMEMORY;
    }

    auto result = factory->QueryInterface(riid, ppvObject);

    factory->Release();

    return result;
}

STDMETHODIMP CClassFactory::QueryInterface(REFIID riid,
                                           void** ppvObject)
{
    static const QITAB qit[] =
    {
        QITABENT(CClassFactory, IClassFactory),
        {0},
    };
    return QISearch(this, qit, riid, ppvObject);
}

STDMETHODIMP_(ULONG) CClassFactory::AddRef()
{
    LONG cRef = InterlockedIncrement(&m_cRef);
    return (ULONG)cRef;
}

STDMETHODIMP_(ULONG) CClassFactory::Release()
{
    LONG cRef = InterlockedDecrement(&m_cRef);
    if (0 == cRef)
        delete this;
    return (ULONG)cRef;
}

STDMETHODIMP CClassFactory::CreateInstance(IUnknown* punkOuter,
                                           REFIID riid,
                                           void** ppvObject)
{
    if (NULL != punkOuter)
        return CLASS_E_NOAGGREGATION;

    return CThumbnailProvider::QueryInterfaceFactory(riid, ppvObject);
}

STDMETHODIMP CClassFactory::LockServer(BOOL fLock)
{
    UNREFERENCED_PARAMETER(fLock);
    return E_NOTIMPL;
}

STDAPI DllGetClassObject(REFCLSID rclsid,
                         REFIID riid,
                         void **ppv)
{
    if (NULL == ppv)
        return E_INVALIDARG;

    if (!IsEqualCLSID(CLSID_SampleThumbnailProvider, rclsid))
        return CLASS_E_CLASSNOTAVAILABLE;

    auto hr = CClassFactory::QueryInterfaceFactory(riid, ppv);
    return hr;
}
