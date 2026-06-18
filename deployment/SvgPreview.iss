; SVG Preview - SVG thumbnails for Windows Explorer, rendered with resvg.
; Based on SVG Explorer Extension (SVG See) by Tibold Kandrai,
; https://github.com/tibold/svg-explorer-extension
#ifndef arch
; Default to x64 if nothing was defined.
#define arch 'x64'
#endif
[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{0C3C0E1A-DDC9-4E6F-9F0B-66FCC9C0437C}
AppMutex=github_sicksartori_svg_preview
AppName="SVG Preview"
AppVersion="2.0.1"
AppVerName="SVG Preview 2.0.1"
AppPublisher="Gabriel Sartori"
AppPublisherURL=https://github.com/SickSartori/svg-preview
AppSupportURL=https://github.com/SickSartori/svg-preview/issues
AppUpdatesURL=https://github.com/SickSartori/svg-preview/releases
DefaultGroupName="SvgPreview"
OutputDir=..\var\installer
OutputBaseFilename="svg_preview_{#arch}"
SetupIconFile=assets\SvgPreview.ico
UninstallDisplayIcon={app}\SvgPreview.ico
Compression=lzma
SolidCompression=yes
ChangesAssociations=yes
; This is to support both IS 5 and 6
#if arch == 'x64'
#ifdef commonpf64
DefaultDirName="{commonpf64}\SvgPreview"
#else
DefaultDirName="{pf64}\SvgPreview"
#endif
#else
#ifdef commonpf32
DefaultDirName="{commonpf32}\SvgPreview"
#else
DefaultDirName="{pf32}\SvgPreview"
#endif
#endif

#if arch == "x64"
ArchitecturesInstallIn64BitMode=x64
#else
ArchitecturesInstallIn64BitMode=
#endif

[Languages]
Name: "en"; MessagesFile: "compiler:Default.isl"; LicenseFile: "..\LICENSE.md"

[Files]
; NOTE: Don't use "Flags: ignoreversion" on any shared system files
Source: "..\var\dist\{#arch}\release\*"; DestDir: "{app}"; Flags: recursesubdirs
Source: "..\var\dist\{#arch}\release\SvgPreview.dll"; DestDir: "{app}"; Flags: regserver
Source: "assets\SvgPreview.ico"; DestDir: "{app}";
; Licenses
Source: "..\var\licenses\resvg-MIT.txt"; DestDir: "{app}\license\";
Source: "..\var\licenses\resvg-APACHE.txt"; DestDir: "{app}\license\";
Source: "..\LICENSE.md"; DestDir: "{app}\license\";

[Code]

// Runs the given uninstaller silently if its registry entry exists.
Procedure UninstallIfPresent(UninstallKey: String);
Var
  ResultCode: Integer;
  Uninstaller: String;
Begin
  If RegQueryStringValue(HKLM, UninstallKey, 'UninstallString', Uninstaller) Then Begin
    Exec(RemoveQuotes(Uninstaller), ' /SILENT', '', SW_SHOWNORMAL, ewWaitUntilTerminated, ResultCode);
  End;
End;

// Automatically uninstalls previously installed versions, then installs the
// VC runtime before the DLL gets registered (issue tibold#75).
// IMPORTANT NOTE: AppIds are hardcoded below, since ExpandConstant did not want to substitute them!
Procedure CurStepChanged(CurStep: TSetupStep);
Var
  ResultCode: Integer;
Begin
  If (CurStep = ssInstall) Then Begin
    // Previous SVG Preview versions
    UninstallIfPresent('Software\Microsoft\Windows\CurrentVersion\Uninstall\{0C3C0E1A-DDC9-4E6F-9F0B-66FCC9C0437C}_is1');
    // Legacy SVG See (upstream), which would otherwise keep a competing
    // thumbnail handler registered for .svg
    UninstallIfPresent('Software\Microsoft\Windows\CurrentVersion\Uninstall\{4CA20D9A-98AC-4DD6-9C16-7449F29AC08A}_is1');
    ExtractTemporaryFile(ExpandConstant('vc_redist.{#arch}.exe'));
    Exec(ExpandConstant('{tmp}\vc_redist.{#arch}.exe'), '/install /passive /norestart', '', SW_SHOWNORMAL, ewWaitUntilTerminated,ResultCode);
  End;
End;
