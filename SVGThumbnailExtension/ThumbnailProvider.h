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

#include <resvg.h>

class CThumbnailProvider : public IThumbnailProvider, IObjectWithSite, IInitializeWithStream
{
private:
    LONG m_cRef;
    IUnknown* m_pSite;
    resvg_render_tree* m_tree;

    ~CThumbnailProvider();

public:
    CThumbnailProvider();

    // Helper
    static HRESULT QueryInterfaceFactory(REFIID, void**);

    //  IUnknown methods
    STDMETHOD(QueryInterface)(REFIID, void**);
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();

    //  IInitializeWithSteam methods
    STDMETHOD(Initialize)(IStream*, DWORD);

    //  IThumbnailProvider methods
    STDMETHOD(GetThumbnail)(UINT, HBITMAP*, WTS_ALPHATYPE*);

    //  IObjectWithSite methods
    STDMETHOD(GetSite)(REFIID, void**);
    STDMETHOD(SetSite)(IUnknown*);
};
