#ifndef SVGTHUMBNAILEXTENSION_GLOBAL_H
#define SVGTHUMBNAILEXTENSION_GLOBAL_H

#include <windows.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <thumbcache.h>
#include <strsafe.h>

STDAPI_(ULONG) DllAddRef();
STDAPI_(ULONG) DllRelease();
STDAPI_(HINSTANCE) DllInstance();

// {4CA20D9A-98AC-4DD6-9C16-7449F29AC08A}
#define szCLSID_SampleThumbnailProvider L"{4CA20D9A-98AC-4DD6-9C16-7449F29AC08A}"
DEFINE_GUID(CLSID_SampleThumbnailProvider,
            0x4ca20d9a, 0x98ac, 0x4dd6, 0x9c, 0x16, 0x74, 0x49, 0xf2, 0x9a, 0xc0, 0x8a);

#endif // SVGTHUMBNAILEXTENSION_GLOBAL_H
