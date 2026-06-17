cmake_minimum_required(VERSION 3.20)

project(SvgPreview VERSION 2.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Metadata embedded into the DLL's Win32 version resource
# (shown in Explorer > Properties > Details).
set(SVGPREVIEW_PRODUCT     "SVG Preview")
set(SVGPREVIEW_DESCRIPTION "SVG thumbnail provider for Windows Explorer")
set(SVGPREVIEW_COMPANY     "Gabriel Sartori")
set(SVGPREVIEW_COPYRIGHT   "Copyright (C) 2026 Gabriel Sartori. LGPL-3.0. Based on SVG Explorer Extension by Tibold Kandrai.")
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/SVGThumbnailExtension/VersionInfo.rc.in
    ${CMAKE_CURRENT_BINARY_DIR}/VersionInfo.rc
    @ONLY)

if(NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(RESVG_ARCH x86)
else()
    set(RESVG_ARCH x64)
endif()

set(RESVG_DIR ${CMAKE_CURRENT_SOURCE_DIR}/SVGThumbnailExtension/thirdparty/resvg)
set(RESVG_LIB ${RESVG_DIR}/lib/${RESVG_ARCH}/resvg.lib)

if(NOT EXISTS ${RESVG_LIB})
    message(FATAL_ERROR
        "resvg static library not found at ${RESVG_LIB}. "
        "Build it first: cargo build --release in resvg/crates/c-api, "
        "then copy target/release/resvg.lib there (see deployment/Build.ps1).")
endif()

# The shell extension DLL
add_library(SvgPreview SHARED
    SVGThumbnailExtension/ClassFactory.cpp
    SVGThumbnailExtension/Main.cpp
    SVGThumbnailExtension/Registry.cpp
    SVGThumbnailExtension/ThumbnailProvider.cpp
    SVGThumbnailExtension/ThumbnailProvider.def
    ${CMAKE_CURRENT_BINARY_DIR}/VersionInfo.rc
)

target_include_directories(SvgPreview PRIVATE ${RESVG_DIR}/include)

# System libraries: the first row is used by the extension itself,
# the second row is required by the static resvg (Rust) runtime.
target_link_libraries(SvgPreview PRIVATE
    ${RESVG_LIB}
    shlwapi advapi32
    ntdll userenv ws2_32 dbghelp
)

target_compile_definitions(SvgPreview PRIVATE
    UNICODE _UNICODE
    $<$<CONFIG:Release>:NDEBUG>
)

# Offline test harness: loads the DLL the same way Explorer does
# (DllGetClassObject -> IInitializeWithStream -> IThumbnailProvider)
# and saves the resulting HBITMAP as a PNG.
add_executable(ThumbnailTestHarness
    test/harness/main.cpp
)

target_compile_definitions(ThumbnailTestHarness PRIVATE UNICODE _UNICODE)

target_link_libraries(ThumbnailTestHarness PRIVATE
    shlwapi ole32 gdiplus uuid
)
