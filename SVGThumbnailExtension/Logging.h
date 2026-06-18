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

#include <windows.h>
#include <sstream>

// Minimal stream-style logger that forwards to OutputDebugStringW.
// View the output with DebugView or a debugger attached to explorer.exe.
class DebugLogLine
{
public:
    ~DebugLogLine()
    {
        m_stream << L"\n";
        OutputDebugStringW(m_stream.str().c_str());
    }

    template <typename T>
    DebugLogLine& operator<<(const T& value)
    {
        m_stream << value;
        return *this;
    }

private:
    std::wostringstream m_stream;
};

#define debugLog DebugLogLine() << L"SvgSee: "
