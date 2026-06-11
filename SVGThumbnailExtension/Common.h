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

// SVG Preview thumbnail provider.
// This CLSID is distinct from the upstream SVG See one on purpose, so both
// products can be installed without clobbering each other's registration.
// {927C8F59-C5EA-488C-995E-A61AF402DF7D}
#define szCLSID_SampleThumbnailProvider L"{927C8F59-C5EA-488C-995E-A61AF402DF7D}"
DEFINE_GUID(CLSID_SampleThumbnailProvider,
            0x927c8f59, 0xc5ea, 0x488c, 0x99, 0x5e, 0xa6, 0x1a, 0xf4, 0x02, 0xdf, 0x7d);

#endif // SVGTHUMBNAILEXTENSION_GLOBAL_H
