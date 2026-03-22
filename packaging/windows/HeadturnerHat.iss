#define MyAppName "Headturner Hat"
#define MyAppVersion "0.1.3"
#define MyAppPublisher "Codex Audio Lab"
#define MyAppExeName "Headturner Hat.vst3"

[Setup]
AppId={{A3C1A11C-1F0C-4DB5-90F4-3B68D02A6A4F}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
DefaultDirName={commoncf}\VST3
DefaultGroupName={#MyAppName}
DisableProgramGroupPage=yes
OutputDir=.
OutputBaseFilename=HeadturnerHat-Windows-Installer
Compression=lzma
SolidCompression=yes
WizardStyle=modern

[Files]
Source: "Headturner Hat.vst3"; DestDir: "{commoncf}\VST3"; Flags: recursesubdirs createallsubdirs ignoreversion

[Icons]
Name: "{group}\Open VST3 Folder"; Filename: "{commoncf}\VST3"

[Run]
Filename: "{commoncf}\VST3"; Description: "Open VST3 folder"; Flags: postinstall shellexec skipifsilent
