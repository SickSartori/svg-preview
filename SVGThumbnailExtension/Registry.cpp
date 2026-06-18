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
#include "Registry.h"
#include "Logging.h"

STDAPI CreateRegistryKey(REGKEY_SUBKEY_AND_VALUE* pKey)
{
    debugLog << "Enter: Create single registry key";
    size_t cbData;
    LPVOID pvData = NULL;
    HRESULT hr = S_OK;

    switch(pKey->dwType)
    {
    case REG_DWORD:
        pvData = (LPVOID)(LPDWORD)&pKey->dwData;
        cbData = sizeof(DWORD);
        break;

    case REG_SZ:
    case REG_EXPAND_SZ:
        hr = StringCbLength((LPCWSTR)pKey->dwData, STRSAFE_MAX_CCH, &cbData);
        if (SUCCEEDED(hr))
        {
            pvData = (LPVOID)(LPCWSTR)pKey->dwData;
            cbData += sizeof(WCHAR);
        }
        break;

    default:
        hr = E_INVALIDARG;
    }

    if (SUCCEEDED(hr))
    {
        debugLog << "Setting key: " << pKey->lpszSubKey;
        LSTATUS status = SHSetValue(pKey->hKey, pKey->lpszSubKey, pKey->lpszValue, pKey->dwType, pvData, (DWORD)cbData);
        if (NOERROR != status)
        {
            hr = HRESULT_FROM_WIN32(status);
            debugLog << "Failed to set registry key.";
        }
    }

    debugLog << "Leave: Create single registry key";
    return hr;
}

STDAPI CreateRegistryKeys(REGKEY_SUBKEY_AND_VALUE* aKeys, ULONG cKeys)
{
    debugLog << "Enter: Create multiple registry key";
    HRESULT hr = S_OK;

    for (ULONG iKey = 0; iKey < cKeys; iKey++)
    {
        HRESULT hrTemp = CreateRegistryKey(&aKeys[iKey]);
        if (FAILED(hrTemp))
        {
            hr = hrTemp;
        }
    }

    debugLog << "Leave: Create multiple registry key";
    return hr;
}

STDAPI DeleteRegistryKeys(REGKEY_SUBKEY* aKeys, ULONG cKeys)
{
    debugLog << "Enter: Delete registry keys";
    HRESULT hr = S_OK;
    LSTATUS status;

    for (ULONG iKey = 0; iKey < cKeys; iKey++)
    {
        debugLog << "Deleting key: " << aKeys[iKey].lpszSubKey;
        status = RegDeleteTree(aKeys[iKey].hKey, aKeys[iKey].lpszSubKey);
        if (NOERROR != status)
        {
            hr = HRESULT_FROM_WIN32(status);
            debugLog << "Failed to delete registry key";
        }
    }
    debugLog << "Leave: Delete registry keys";
    return hr;
}
