; OpenSheet Inno Setup Script
; Produces a professional Windows installer: OpenSheet-Setup-x64.exe

#define AppName      "OpenSheet"
#define AppVersion   "1.0.0"
#define AppPublisher "OpenSheet Project"
#define AppURL       "https://github.com/opensheet/opensheet"
#define AppExeName   "opensheet.exe"
#define BuildDir     "..\..\build\bin\Release"

[Setup]
AppId={{A3F2B4C1-9E7D-4F0A-8B3C-2D5E6F7A8B9C}
AppName={#AppName}
AppVersion={#AppVersion}
AppVerName={#AppName} {#AppVersion}
AppPublisher={#AppPublisher}
AppPublisherURL={#AppURL}
AppSupportURL={#AppURL}/issues
AppUpdatesURL={#AppURL}/releases
DefaultDirName={autopf}\{#AppName}
DefaultGroupName={#AppName}
AllowNoIcons=yes
LicenseFile=..\..\LICENSE
OutputDir=output
OutputBaseFilename=OpenSheet-Setup-{#AppVersion}-x64
SetupIconFile=..\..\resources\icons\logo.ico
Compression=lzma2/ultra64
SolidCompression=yes
WizardStyle=modern
ArchitecturesInstallIn64BitMode=x64
MinVersion=10.0
PrivilegesRequired=lowest
PrivilegesRequiredOverridesAllowed=dialog
UninstallDisplayIcon={app}\{#AppExeName}
UninstallDisplayName={#AppName} {#AppVersion}
VersionInfoVersion={#AppVersion}
VersionInfoCompany={#AppPublisher}
VersionInfoDescription={#AppName} Spreadsheet Application
ChangesAssociations=yes

[Languages]
Name: "english";    MessagesFile: "compiler:Default.isl"
Name: "french";     MessagesFile: "compiler:Languages\French.isl"
Name: "german";     MessagesFile: "compiler:Languages\German.isl"

[Tasks]
Name: "desktopicon";     Description: "{cm:CreateDesktopIcon}";      GroupDescription: "{cm:AdditionalIcons}"
Name: "quicklaunchicon"; Description: "{cm:CreateQuickLaunchIcon}";  GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked
Name: "fileassoc";       Description: "Associate .opensheet files with {#AppName}"; GroupDescription: "File associations:"
Name: "fileassocxlsx";   Description: "Associate .xlsx files with {#AppName} (optional)"; GroupDescription: "File associations:"; Flags: unchecked

[Files]
; Main executable
Source: "{#BuildDir}\{#AppExeName}";          DestDir: "{app}";             Flags: ignoreversion

; Qt runtime DLLs (windeployqt output)
Source: "{#BuildDir}\Qt6Core.dll";            DestDir: "{app}";             Flags: ignoreversion
Source: "{#BuildDir}\Qt6Gui.dll";             DestDir: "{app}";             Flags: ignoreversion
Source: "{#BuildDir}\Qt6Widgets.dll";         DestDir: "{app}";             Flags: ignoreversion
Source: "{#BuildDir}\Qt6Charts.dll";          DestDir: "{app}";             Flags: ignoreversion
Source: "{#BuildDir}\Qt6Svg.dll";             DestDir: "{app}";             Flags: ignoreversion
Source: "{#BuildDir}\Qt6Network.dll";         DestDir: "{app}";             Flags: ignoreversion
Source: "{#BuildDir}\Qt6Xml.dll";             DestDir: "{app}";             Flags: ignoreversion
Source: "{#BuildDir}\Qt6Sql.dll";             DestDir: "{app}";             Flags: ignoreversion
Source: "{#BuildDir}\Qt6PrintSupport.dll";    DestDir: "{app}";             Flags: ignoreversion
Source: "{#BuildDir}\platforms\*";            DestDir: "{app}\platforms";   Flags: ignoreversion recursesubdirs
Source: "{#BuildDir}\styles\*";               DestDir: "{app}\styles";      Flags: ignoreversion recursesubdirs
Source: "{#BuildDir}\imageformats\*";         DestDir: "{app}\imageformats"; Flags: ignoreversion recursesubdirs
Source: "{#BuildDir}\sqldrivers\*";           DestDir: "{app}\sqldrivers";  Flags: ignoreversion recursesubdirs

; Visual C++ runtime
Source: "{#BuildDir}\vc_redist.x64.exe";      DestDir: "{tmp}";             Flags: deleteafterinstall

; App data files
Source: "..\..\themes\*";                     DestDir: "{app}\themes";      Flags: ignoreversion recursesubdirs createallsubdirs
Source: "..\..\localization\*";               DestDir: "{app}\localization"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "..\..\resources\*";                  DestDir: "{app}\resources";   Flags: ignoreversion recursesubdirs createallsubdirs
Source: "..\..\addons\*";                     DestDir: "{app}\addons";      Flags: ignoreversion recursesubdirs createallsubdirs
Source: "..\..\config\*";                     DestDir: "{app}\config";      Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{group}\{#AppName}";                   Filename: "{app}\{#AppExeName}"
Name: "{group}\Uninstall {#AppName}";         Filename: "{uninstallexe}"
Name: "{autodesktop}\{#AppName}";             Filename: "{app}\{#AppExeName}";  Tasks: desktopicon
Name: "{userappdata}\Microsoft\Internet Explorer\Quick Launch\{#AppName}"; \
      Filename: "{app}\{#AppExeName}";  Tasks: quicklaunchicon

[Registry]
; .opensheet file association
Root: HKA; Subkey: "Software\Classes\.opensheet";       ValueType: string; ValueName: ""; ValueData: "OpenSheet.Document";    Flags: uninsdeletevalue; Tasks: fileassoc
Root: HKA; Subkey: "Software\Classes\OpenSheet.Document"; ValueType: string; ValueName: ""; ValueData: "OpenSheet Workbook"; Flags: uninsdeletekey;    Tasks: fileassoc
Root: HKA; Subkey: "Software\Classes\OpenSheet.Document\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: "{app}\{#AppExeName},0"; Tasks: fileassoc
Root: HKA; Subkey: "Software\Classes\OpenSheet.Document\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\{#AppExeName}"" ""%1"""; Tasks: fileassoc

; Optional .xlsx association
Root: HKA; Subkey: "Software\Classes\.xlsx\OpenWithProgids"; ValueType: string; ValueName: "OpenSheet.Document"; ValueData: ""; Flags: uninsdeletevalue; Tasks: fileassocxlsx

[Run]
; Install VC++ runtime silently
Filename: "{tmp}\vc_redist.x64.exe"; Parameters: "/quiet /norestart"; StatusMsg: "Installing Visual C++ Runtime…"; Flags: waituntilterminated

; Launch app after install
Filename: "{app}\{#AppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(AppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent

[UninstallDelete]
Type: filesandordirs; Name: "{app}\temp"
Type: filesandordirs; Name: "{app}\logs"

[Code]
function InitializeSetup(): Boolean;
begin
  Result := True;
end;
