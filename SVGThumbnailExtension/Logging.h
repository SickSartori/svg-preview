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
