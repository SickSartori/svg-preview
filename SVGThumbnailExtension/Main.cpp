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
#define INITGUID
#include "Logging.h"
#include "Registry.h"

HINSTANCE g_hinstDll = NULL;
LONG g_cRef = 0;

BOOL APIENTRY DllMain(HINSTANCE hinstDll,
                      DWORD dwReason,
                      LPVOID pvReserved)
{
    UNREFERENCED_PARAMETER(pvReserved);
    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:
        debugLog << "DLL_PROCESS_ATTACH";
        g_hinstDll = hinstDll;
        // No per-thread initialization is needed.
        DisableThreadLibraryCalls(hinstDll);
        break;
    }
    return TRUE;
}

STDAPI_(HINSTANCE) DllInstance()
{
    return g_hinstDll;
}

STDAPI DllCanUnloadNow()
{
    return g_cRef ? S_FALSE : S_OK;
}

STDAPI_(ULONG) DllAddRef()
{
    LONG cRef = InterlockedIncrement(&g_cRef);
    return cRef;
}

STDAPI_(ULONG) DllRelease()
{
    LONG cRef = InterlockedDecrement(&g_cRef);
    if (0 > cRef)
        cRef = 0;
    return cRef;
}

STDAPI DllRegisterServer()
{
    debugLog << "Enter: DLLRegisterServer";
    WCHAR szModule[MAX_PATH];

    ZeroMemory(szModule, sizeof(szModule));
    GetModuleFileName(g_hinstDll, szModule, ARRAYSIZE(szModule));

    REGKEY_SUBKEY_AND_VALUE keys[] = {
        {HKEY_CLASSES_ROOT, L"CLSID\\" szCLSID_SampleThumbnailProvider, NULL, REG_SZ, (DWORD_PTR)L"SvgPreview"},
        {HKEY_CLASSES_ROOT, L"CLSID\\" szCLSID_SampleThumbnailProvider L"\\InprocServer32", NULL, REG_SZ, (DWORD_PTR)szModule},
        {HKEY_CLASSES_ROOT, L"CLSID\\" szCLSID_SampleThumbnailProvider L"\\InprocServer32", L"ThreadingModel", REG_SZ, (DWORD_PTR)L"Apartment"},
        {HKEY_CLASSES_ROOT, L".SVG\\shellex\\{E357FCCD-A995-4576-B01F-234630154E96}", NULL, REG_SZ, (DWORD_PTR)szCLSID_SampleThumbnailProvider},
        {HKEY_CLASSES_ROOT, L".SVGZ\\shellex\\{E357FCCD-A995-4576-B01F-234630154E96}", NULL, REG_SZ, (DWORD_PTR)szCLSID_SampleThumbnailProvider}
    };


    auto result = CreateRegistryKeys(keys, ARRAYSIZE(keys));

    debugLog << "Leaving: DLLRegisterServer";

    return result;
}

STDAPI DllUnregisterServer()
{
    debugLog << "Enter: DLLUnregisterServer";

    REGKEY_SUBKEY keys[] = {{HKEY_CLASSES_ROOT, L"CLSID\\" szCLSID_SampleThumbnailProvider}};
    auto result = DeleteRegistryKeys(keys, ARRAYSIZE(keys));

    debugLog << "Leaving: DLLUnregisterServer";
    return result;
}
