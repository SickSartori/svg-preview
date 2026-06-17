[CmdletBinding()]
Param(
    [Parameter()]
    [string] $ProjectName = 'SvgPreview',

    [Parameter()]
    [ValidateSet('release', 'debug')]
    [string] $Configuration = 'release',

    [Parameter()]
    [ValidateSet('x86', 'x64')]
    [string] $Architecture = 'x64',

    # Tag of the resvg release used as the rendering engine.
    [Parameter()]
    [string] $ResvgVersion = 'v0.47.0',

    [Parameter()]
    [string] $ResvgRepository = 'https://github.com/linebender/resvg.git',

    # Which part of the build to run.
    #   All       - resvg + DLL + installer (local one-shot build)
    #   Dll       - resvg + DLL only (CI stops here to sign the DLL)
    #   Installer - vc_redist + licenses + installer, assuming the (signed)
    #               DLL is already present in the dist folder
    [Parameter()]
    [ValidateSet('All', 'Dll', 'Installer')]
    [string] $Stage = 'All',

    [Parameter()]
    [string] $InnoSetupPath = 'C:\Program Files (x86)\Inno Setup 6'
)

$ErrorActionPreference = 'stop'

Import-Module (Join-Path $PSScriptRoot 'Modules/Utils.psd1')

$rootFolder = Resolve-Path (Join-Path $PSScriptRoot '..')

$distDir = Join-Path $rootFolder "var/dist/$Architecture/$Configuration"
$buildDir = Join-Path $rootFolder "var/build/$Architecture"
$resvgSrcDir = Join-Path $rootFolder 'var/resvg'
$resvgVendorDir = Join-Path $rootFolder 'SVGThumbnailExtension/thirdparty/resvg'
$licenseDir = Join-Path $rootFolder 'var/licenses'
$installerDir = Join-Path $rootFolder 'var/installer'
$installerPath = Join-Path $installerDir "svg_preview_$Architecture.exe"

$cmakeArchMap = @{
    'x86' = 'Win32';
    'x64' = 'x64';
}
$rustTargetMap = @{
    'x86' = 'i686-pc-windows-msvc';
    'x64' = 'x86_64-pc-windows-msvc';
}
$cmakeConfigMap = @{
    'release' = 'Release';
    'debug' = 'Debug';
}

function Find-CMake {
    $cmake = Get-Command cmake -ErrorAction SilentlyContinue
    if ($cmake) {
        return $cmake.Source
    }

    $vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    if (-not (Test-Path $vswhere)) {
        throw 'cmake was not found on PATH and Visual Studio could not be located (vswhere missing).'
    }

    $vsPath = & $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
    if (-not $vsPath) {
        throw 'No Visual Studio installation with C++ tools was found.'
    }

    $bundled = Join-Path $vsPath 'Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe'
    if (-not (Test-Path $bundled)) {
        throw "Visual Studio found at '$vsPath' but it does not include CMake. Install the 'C++ CMake tools' component."
    }

    return $bundled
}

function Initialize-Environment {

    if ($Stage -ne 'Installer' -and -not (Get-Command cargo -ErrorAction SilentlyContinue)) {
        $cargoBin = Join-Path $env:USERPROFILE '.cargo\bin'
        if (Test-Path (Join-Path $cargoBin 'cargo.exe')) {
            $env:Path = "$cargoBin;$env:Path"
        }
        else {
            throw 'cargo (Rust) is required to build the resvg rendering engine. Install it from https://rustup.rs/.'
        }
    }

    if ($Stage -ne 'Installer') {
        $script:cmake = Find-CMake
        Write-Verbose "Using CMake at: $script:cmake"
    }

    if ($Stage -ne 'Dll') {
        # Setup "Inno Setup" build environment
        Use-InnoSetup -InstallPath $InnoSetupPath
    }
}

function Build-Resvg {

    Write-Verbose "Building resvg $ResvgVersion for $Architecture."

    if (-not (Test-Path (Join-Path $resvgSrcDir '.git'))) {
        git clone --depth 1 --branch $ResvgVersion $ResvgRepository $resvgSrcDir
        Assert-LastExitCode 'Failed to clone resvg'
    }
    else {
        git -C $resvgSrcDir fetch --depth 1 origin tag $ResvgVersion
        Assert-LastExitCode 'Failed to fetch resvg'
        git -C $resvgSrcDir checkout $ResvgVersion
        Assert-LastExitCode 'Failed to checkout resvg version'
    }

    $rustTarget = $rustTargetMap[$Architecture]

    rustup target add $rustTarget
    Assert-LastExitCode 'Failed to add rust target'

    Push-Location (Join-Path $resvgSrcDir 'crates/c-api')
    try {
        cargo build --release --target $rustTarget
        Assert-LastExitCode 'Failed to build resvg'
    }
    finally {
        Pop-Location
    }

    # Vendor the build products where CMake expects them.
    $libDir = Join-Path $resvgVendorDir "lib/$Architecture"
    New-Item -Path $libDir -ItemType Directory -Force | Out-Null
    New-Item -Path (Join-Path $resvgVendorDir 'include') -ItemType Directory -Force | Out-Null

    Copy-Item (Join-Path $resvgSrcDir "target/$rustTarget/release/resvg.lib") (Join-Path $libDir 'resvg.lib') -Force
    Copy-Item (Join-Path $resvgSrcDir 'crates/c-api/resvg.h') (Join-Path $resvgVendorDir 'include/resvg.h') -Force
}

function Build-Application {

    Write-Verbose 'Building application.'

    New-Item -Path $distDir -ItemType Directory -Force | Out-Null
    New-Item -Path $buildDir -ItemType Directory -Force | Out-Null

    $cmakeConfig = $cmakeConfigMap[$Configuration]

    & $script:cmake -S $rootFolder -B $buildDir -G 'Visual Studio 17 2022' -A $cmakeArchMap[$Architecture]
    Assert-LastExitCode 'Failed to configure the project'

    & $script:cmake --build $buildDir --config $cmakeConfig
    Assert-LastExitCode 'Failed to build the project'

    Copy-Item (Join-Path $buildDir "$cmakeConfig/SvgPreview.dll") $distDir -Force
}

function Publish-Application {

    Write-Verbose 'Downloading the Visual C++ runtime redistributable.'

    # The installer runs vc_redist before registering the DLL (issue #75).
    $vcRedist = Join-Path $distDir "vc_redist.$Architecture.exe"
    if (-not (Test-Path $vcRedist)) {
        Invoke-WebRequest -Uri "https://aka.ms/vs/17/release/vc_redist.$Architecture.exe" -OutFile $vcRedist
    }

    Write-Verbose 'Gathering licenses'

    New-Item -Path $licenseDir -ItemType Directory -Force | Out-Null
    Copy-Item -Path (Join-Path $resvgSrcDir 'LICENSE-MIT') -Destination (Join-Path $licenseDir 'resvg-MIT.txt') -Force
    Copy-Item -Path (Join-Path $resvgSrcDir 'LICENSE-APACHE') -Destination (Join-Path $licenseDir 'resvg-APACHE.txt') -Force
}

function Build-Installer {

    Write-Verbose 'Building installer'

    Push-Location $rootFolder
    try {
        $issFile = Join-Path $rootFolder "deployment/${ProjectName}.iss"
        iscc "/darch=$Architecture" "$issFile" | Write-Verbose
        Assert-LastExitCode 'Failed to build installer'
    }
    finally {
        Pop-Location
    }
}

Initialize-Environment

# Authenticode signing is performed by the CI between these stages
# (the DLL is signed before it is packaged, the installer after it is built),
# which is why the build can be run one stage at a time.
if ($Stage -ne 'Installer') {
    Build-Resvg
    Build-Application
}
if ($Stage -ne 'Dll') {
    Publish-Application
    Build-Installer
}
