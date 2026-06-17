# Code signing

Release installers are Authenticode-signed through [SignPath](https://signpath.org),
which is free for open-source projects. Signing removes the SmartScreen /
Microsoft Defender "unknown publisher" warnings and the `Wacatac.*!ml`
false-positive that unsigned installers trigger.

Signing runs in [`.github/workflows/release.yml`](../.github/workflows/release.yml)
when a GitHub release is published. The DLL is signed first, packaged into the
installer, and the installer is then signed too.

## One-time setup

1. **Sign up for the SignPath OSS plan** at <https://signpath.org/> with the
   GitHub account that owns this repository, and request the free
   *Open Source* plan for `SickSartori/svg-preview`. Approval is manual and
   can take a few days.

2. In the SignPath organization, create:
   - a **Project** with slug `svg-preview`;
   - two **artifact configurations**: `dll` (a single Authenticode PE file)
     and `installer` (a single Authenticode PE file);
   - a **signing policy** with slug `release-signing` using the
     SignPath-provided OSS certificate.

3. In **GitHub → repository → Settings**:
   - *Secrets and variables → Actions → Secrets*: add `SIGNPATH_API_TOKEN`
     (a SignPath CI user API token).
   - *Secrets and variables → Actions → Variables*: add `SIGNPATH_ORG_ID`
     (your SignPath organization id) and set `SIGNING_ENABLED` to `true`.

Until `SIGNING_ENABLED` is `true`, the release workflow is skipped and
releases stay unsigned (the `build.yml` workflow still produces an unsigned
installer artifact for testing).

## Cutting a signed release

1. Bump the version in `CMakeLists.txt` (`project(... VERSION ...)`) and
   `deployment/SvgPreview.iss`.
2. Tag and create a GitHub release for that version.
3. The `release` workflow builds, signs and attaches `svg_preview_x64.exe`
   to the release.
4. Update the winget manifest to point at the new signed installer and its
   SHA256.
