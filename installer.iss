; Inno Setup script - builds a real one-click installer for Scale Operator
#define MyAppName "Scale Operator"
#define MyAppVersion "1.0.0"
#define MyAppPublisher "HomeBrew"

[Setup]
AppId={{B7E2A6B4-9C1D-4E3A-8F2B-5D7C3A1E9F60}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
DefaultDirName={commoncf64}\VST3\{#MyAppName}.vst3
DisableDirPage=yes
DisableProgramGroupPage=yes
PrivilegesRequired=admin
ArchitecturesInstallIn64BitMode=x64compatible
OutputDir=installer_out
OutputBaseFilename=ScaleOperatorSetup
Compression=lzma
SolidCompression=yes
WizardStyle=modern
UninstallDisplayIcon={app}\Scale Operator.vst3

[Files]
Source: "dist\Scale Operator.vst3\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "vc_redist.x64.exe"; DestDir: "{tmp}"; Flags: deleteafterinstall

[Run]
Filename: "{tmp}\vc_redist.x64.exe"; Parameters: "/install /quiet /norestart"; StatusMsg: "Installing required runtime components..."; Flags: waituntilterminated

[UninstallDelete]
Type: filesandordirs; Name: "{app}"
