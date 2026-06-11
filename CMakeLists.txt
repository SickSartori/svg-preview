cmake_minimum_required(VERSION 3.20)

project(SvgSee LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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
add_library(SvgSee SHARED
    SVGThumbnailExtension/ClassFactory.cpp
    SVGThumbnailExtension/Main.cpp
    SVGThumbnailExtension/Registry.cpp
    SVGThumbnailExtension/ThumbnailProvider.cpp
    SVGThumbnailExtension/ThumbnailProvider.def
)

target_include_directories(SvgSee PRIVATE ${RESVG_DIR}/include)

# System libraries: the first row is used by the extension itself,
# the second row is required by the static resvg (Rust) runtime.
target_link_libraries(SvgSee PRIVATE
    ${RESVG_LIB}
    shlwapi advapi32
    ntdll userenv ws2_32 dbghelp
)

target_compile_definitions(SvgSee PRIVATE
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
