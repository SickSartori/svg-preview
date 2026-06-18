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

class CClassFactory : public IClassFactory
{
private:
    LONG m_cRef;

    ~CClassFactory();

public:
    CClassFactory();

    // Helper
    static HRESULT QueryInterfaceFactory(REFIID, void**);

    //  IUnknown methods
    STDMETHOD(QueryInterface)(REFIID, void**);
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();

    //  IClassFactory methods
    STDMETHOD(CreateInstance)(IUnknown*, REFIID, void**);
    STDMETHOD(LockServer)(BOOL);
};
