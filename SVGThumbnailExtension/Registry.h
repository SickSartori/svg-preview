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
#pragma once

#include "Common.h"

typedef struct _REGKEY_SUBKEY
{
    HKEY hKey;
    LPCWSTR lpszSubKey;
} REGKEY_SUBKEY;

typedef struct _REGKEY_SUBKEY_AND_VALUE
{
    HKEY hKey;
    LPCWSTR lpszSubKey;
    LPCWSTR lpszValue;
    DWORD dwType;
    DWORD_PTR dwData;
} REGKEY_SUBKEY_AND_VALUE;

/**
 * @brief Creates or updates registry keys
 *
 * @param aKeys Registry keys and values to update
 * @param cKeys The number of keys in the aKeys array.
 */
STDAPI CreateRegistryKeys(REGKEY_SUBKEY_AND_VALUE* aKeys, ULONG cKeys);

/**
 * @brief Deletes the specified registry keys
 *
 * @param aKeys Registry keys to delete
 * @param cKeys The number of keys in the aKeys array.
 */
STDAPI DeleteRegistryKeys(REGKEY_SUBKEY* aKeys, ULONG cKeys);
