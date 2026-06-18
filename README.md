<p align="center">
  <img src="docs/logo.png" alt="SVG Preview" width="128" height="128">
</p>

<h1 align="center">SVG Preview for Windows Explorer</h1>

<p align="center">
  <a href="https://github.com/SickSartori/svg-preview/actions/workflows/build.yml"><img src="https://github.com/SickSartori/svg-preview/actions/workflows/build.yml/badge.svg" alt="build"></a>
</p>

SVG thumbnails in Windows Explorer, rendered with [resvg](https://github.com/linebender/resvg) for accurate, browser-grade results.

Based on [SVG Explorer Extension (SVG See)](https://github.com/tibold/svg-explorer-extension) by [Tibold Kandrai](https://github.com/tibold) - all credit for the original extension goes to him and its contributors. This fork replaces the QtSvg rendering engine, which is the part that could not be fixed upstream.

## Why this fork

QtSvg, the engine used by SVG See, only implements the SVG Tiny profile: `clipPath`, masks and filters are ignored (verified up to Qt 6.11). Any SVG using them - including most exports from Illustrator, Affinity or Inkscape - renders wrong or not at all. That is the root cause of upstream issues [#118](https://github.com/tibold/svg-explorer-extension/issues/118), [#119](https://github.com/tibold/svg-explorer-extension/issues/119), [#123](https://github.com/tibold/svg-explorer-extension/issues/123) and [#125](https://github.com/tibold/svg-explorer-extension/issues/125).

This fork renders through resvg instead, statically linked into a single ~3 MB DLL (no Qt runtime to ship):

| | QtSvg (SVG See) | resvg (SVG Preview) |
|---|---|---|
| clipPath + `<use>` + embedded image | ![QtSvg render](docs/comparison/clip-path-use-image-qtsvg.png) | ![resvg render](docs/comparison/clip-path-use-image-resvg.png) |

The same clip path that QtSvg ignores (left, the image fills the whole square) is applied correctly by resvg (right). Masks, filters, `<use>` forward references, embedded raster images and gzipped `.svgz` all work too.

## Installation

### winget (recommended)

    winget install SickSartori.SvgPreview

### Manual

From _[Releases](https://github.com/SickSartori/svg-preview/releases)_ download and run the appropriate binary for your system. There are no further actions required after installation.

> Make sure you download the right architecture (the 32 bit installer will run on a 64 bit system, but the extension will not function).

Installing SVG Preview automatically removes previous versions, including the original SVG See if present.

### Code signing

Release installers are Authenticode-signed. Free code signing is provided by [SignPath.io](https://signpath.io), with a certificate issued by the [SignPath Foundation](https://signpath.org). See [deployment/SIGNING.md](deployment/SIGNING.md) for details.

## Troubleshooting

----
> Thumbnails do no show after installation

### Method 1:

This may happen if the thumbnail's are disabled in the system. To verify that it is indeed turned on:

* Open the start menu
* Search for `File Explorer Options` and open it
* Under the `View` tab make sure that the `Always show icons, never thumbnails` is __unchecked__

### Method 2:

This may happen if the system already contains cached thumbnails for the SVGs you are trying to view. This can be fixed by clearing the system's thumbnail cache.

* Open the start menu
* Search for "Disk cleanup" and open it
* In the dialog there is a list of items that can be cleaned. Select `Thumbnails` at the end of the list. You may unselect the rest or leave the default selection.
* Click `OK`

### Method 3:

Kill `explorer.exe` and delete the icon cache manually
([ref](https://superuser.com/questions/342052/how-to-get-svg-thumbnails-in-windows-explorer)):
   
    TASKKILL /IM explorer* /F
    DEL "%localappdata%\IconCache.db" /A
    explorer.exe

If neither of the above helped please open an issue on our github page.

----
> An error is thrown during the installation.

Please open an issue on our github page, and include a screen shot and the exact error message.

### Automatic builds
Development installers are created from every commit by [GitHub Actions](https://github.com/SickSartori/svg-preview/actions): open a green build and download the `svg_preview_x64` artifact. Every build also renders all the test SVGs through the freshly built DLL and uploads the resulting thumbnails, so rendering regressions are visible at a glance.

## Developer Build Environment

- MS Visual Studio 2022 (Community or Build Tools) with the *C++ desktop* workload
  and the *C++ CMake tools* component
- [Rust](https://rustup.rs/) (stable, MSVC toolchain) - used to build resvg
- Inno Setup v6 (only needed to build the installer)
- Git

[winget](https://learn.microsoft.com/windows/package-manager/winget/) installation:

    winget install Microsoft.VisualStudio.2022.BuildTools
    winget install Rustlang.Rustup
    winget install JRSoftware.InnoSetup
    winget install Git.Git

[Chocolatey](https://chocolatey.org/) installation:

    choco install visualstudio2022buildtools
    choco install visualstudio2022-workload-vctools
    choco install rustup.install innosetup git

With either package manager, make sure the Visual Studio install includes the
*Desktop development with C++* workload and the *C++ CMake tools* component
(add them from the Visual Studio Installer if needed).

**Quick start** after developer env is set:

    git clone https://github.com/SickSartori/svg-preview.git
    cd svg-preview
    pwsh .\deployment\Build.ps1 -Verbose -Architecture x64

The build script clones and compiles resvg at a pinned release, builds the
DLL with CMake and packages the installer.

To validate thumbnails without registering the extension, the build also produces
`ThumbnailTestHarness.exe`, which drives the DLL exactly like Explorer does:

    ThumbnailTestHarness.exe SvgPreview.dll input.svg output.png 256

## History
Tibold Kandrai started the project in 2012, first on Google Code, Codeplex. Life happened and Tibold didn't have time to work on it any more, though the extension continued to work more than it didn't so people kept using it. 

In 2017 Codeplex shut down and turned into a read-only warehouse. Matt Wilkie imported the project to GitHub and continued to maintain the project as best as a python-not-c++ guy could. The extension continued to work more than not, though the problems started to add up as Windows continued to evolve and change underfoot.

In late 2019 a lucky confluence of stubborn brute force learning on Matt's part and newly active and knowledgeable contributors (Daniel, Simon, Voodoo) revived the feared soon-to-be-comatose project. Bugs were fixed and automatic binary builds came into being. Life rebounded. Right on the heels of this, Tibold regained attention time for side-projects and again assumed the project owner mantle.

On 1st of January, 2020 version v1.0.0 was released including all bug fixes and up to date dependencies. Let's see where the rest of the year takes us. :-)

In 2026 this fork, SVG Preview, replaced the QtSvg engine with resvg after tracking down why SVGs with clip paths had been rendering wrong for years. Everything else - the shell extension design, the installer, the project itself - stands on the shoulders of Tibold's work and that of all the contributors above.

## Contributors ✨

Thank you's for helping make this a better project _([emoji key](https://allcontributors.org/docs/en/emoji-key))_:

* [resvg](https://github.com/linebender/resvg) - SVG rendering engine
* [Qt](https://www.qt.io/) - dev platform and libraries
* [Jeremy@urk](https://www.codemonkeycodes.com/2010/01/11/ithumbnailprovider-re-visited/) - initial example
* [Tibold Kandrai](https://github.com/tibold) - Project creator and primary developer

<!-- ALL-CONTRIBUTORS-LIST:START - Do not remove or modify this section -->
<!-- prettier-ignore-start -->
<!-- markdownlint-disable -->
<table>
  <tr>
    <td align="center"><a href="http://telcontar.net/"><img src="https://avatars3.githubusercontent.com/u/5874930?v=4" width="32px;" alt=""/><br /><sub><b>Daniel Beardsmore</b></sub></a><br /><a href="https://github.com/tibold/svg-explorer-extension/commits?author=Daniel-Beardsmore" title="Code">💻</a></td>
    <td align="center"><a href="https://github.com/GitMensch"><img src="https://avatars3.githubusercontent.com/u/6699539?v=4" width="32px;" alt=""/><br /><sub><b>Simon Sobisch</b></sub></a><br /><a href="https://github.com/tibold/svg-explorer-extension/commits?author=GitMensch" title="Documentation">📖</a> <a href="https://github.com/tibold/svg-explorer-extension/commits?author=GitMensch" title="Code">💻</a></td>
    <td align="center"><a href="https://github.com/voodoo66"><img src="https://avatars1.githubusercontent.com/u/14852960?s=400&v=4" width="32px;" alt=""/><br /><sub><b></b></sub></a><br /><a href="https://github.com/tibold/svg-explorer-extension/commits?author=voodoo66" title="Code">💻</a></td>
  </tr>
  <tr>
    <td align="center"><a href="https://github.com/tibold"><img src="https://avatars2.githubusercontent.com/u/1974659?v=4" width="32px;" alt=""/><br /><sub><b>Tibold Kandrai</b></sub></a><br /><a href="https://github.com/tibold/svg-explorer-extension/commits?author=tibold" title="Tests">⚠️</a> <a href="#ideas-tibold" title="Ideas, Planning, & Feedback">🤔</a> <a href="https://github.com/tibold/svg-explorer-extension/commits?author=tibold" title="Code">💻</a> <a href="#maintenance-tibold" title="Maintenance">🚧</a> <a href="https://github.com/tibold/svg-explorer-extension/commits?author=tibold" title="Documentation">📖</a></td>
    <td align="center"><a href="http://www.maphew.com"><img src="https://avatars3.githubusercontent.com/u/486200?v=4" width="32px;" alt=""/><br /><sub><b>Matt Wilkie</b></sub></a><br /><a href="#maintenance-maphew" title="Maintenance">🚧</a></td>
  </tr>
</table>

<!-- markdownlint-enable -->
<!-- prettier-ignore-end -->
<!-- ALL-CONTRIBUTORS-LIST:END -->

[![All Contributors](https://img.shields.io/badge/all_contributors-5-orange.svg?style=flat-square)](#contributors)  
This project follows the [all-contributors](https://github.com/all-contributors/all-contributors) specification. Contributions of all kinds welcome (code, docs, user support, ...).
