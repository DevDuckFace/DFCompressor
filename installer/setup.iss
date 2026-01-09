; DFCompressor Installer Script
; Inno Setup 6.x

#define MyAppName "DFCompressor"
#define MyAppVersion "1.0.0"
#define MyAppPublisher "DuckForge"
#define MyAppURL "https://github.com/duckforge/dfcompressor"
#define MyAppExeName "DFCompressor.exe"
#define MyAppAssocName MyAppName + " Media File"
#define MyAppAssocExt ".dfproj"
#define MyAppAssocKey StringChange(MyAppAssocName, " ", "") + MyAppAssocExt

[Setup]
; App info
AppId={{D8F3C5A2-9E4D-4B6C-8A2F-1C5E3F7A9B8D}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppVerName={#MyAppName} {#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}

; Directories
DefaultDirName={autopf}\{#MyAppName}
DefaultGroupName={#MyAppName}
DisableProgramGroupPage=yes
OutputDir=output
OutputBaseFilename=DFCompressor_Setup_{#MyAppVersion}

; Appearance - Using the duck icon
SetupIconFile=..\resources\icons\app_icon.ico
WizardStyle=modern
WizardSizePercent=120

; Compression
Compression=lzma2/ultra64
SolidCompression=yes
LZMAUseSeparateProcess=yes

; Privileges
PrivilegesRequired=admin
PrivilegesRequiredOverridesAllowed=dialog

; Other
UninstallDisplayIcon={app}\{#MyAppExeName}
UninstallDisplayName={#MyAppName}
AllowNoIcons=yes
LicenseFile=LICENSE.txt
InfoAfterFile=README.txt

; Minimum requirements
MinVersion=10.0

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"
Name: "portuguese"; MessagesFile: "compiler:Languages\Portuguese.isl"
Name: "brazilianportuguese"; MessagesFile: "compiler:Languages\BrazilianPortuguese.isl"
Name: "spanish"; MessagesFile: "compiler:Languages\Spanish.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked
Name: "quicklaunchicon"; Description: "{cm:CreateQuickLaunchIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked; OnlyBelowVersion: 6.1
Name: "contextmenu"; Description: "Add 'Convert with DFCompressor' to context menu"; GroupDescription: "Shell Integration:"

[Files]
; Main executable
Source: "..\build\bin\{#MyAppExeName}"; DestDir: "{app}"; Flags: ignoreversion

; Dependencies (Qt DLLs, plugins, etc. already copied to bin by build script)
Source: "..\build\bin\*.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\build\bin\platforms\*"; DestDir: "{app}\platforms"; Flags: ignoreversion recursesubdirs
Source: "..\build\bin\styles\*"; DestDir: "{app}\styles"; Flags: ignoreversion recursesubdirs
Source: "..\build\bin\imageformats\*"; DestDir: "{app}\imageformats"; Flags: ignoreversion recursesubdirs
Source: "..\build\bin\multimedia\*"; DestDir: "{app}\multimedia"; Flags: ignoreversion recursesubdirs
Source: "..\build\bin\iconengines\*"; DestDir: "{app}\iconengines"; Flags: ignoreversion recursesubdirs
Source: "..\build\bin\tls\*"; DestDir: "{app}\tls"; Flags: ignoreversion recursesubdirs

; FFmpeg binaries
Source: "..\third_party\ffmpeg\bin\*.exe"; DestDir: "{app}\ffmpeg\bin"; Flags: ignoreversion skipifsourcedoesntexist
Source: "..\third_party\ffmpeg\bin\*.dll"; DestDir: "{app}\ffmpeg\bin"; Flags: ignoreversion skipifsourcedoesntexist

; libvips binaries
Source: "..\third_party\vips\bin\*.exe"; DestDir: "{app}\vips\bin"; Flags: ignoreversion skipifsourcedoesntexist
Source: "..\third_party\vips\bin\*.dll"; DestDir: "{app}\vips\bin"; Flags: ignoreversion skipifsourcedoesntexist

; VC++ Runtime
Source: "..\third_party\redist\VC_redist.x64.exe"; DestDir: "{tmp}"; Flags: deleteafterinstall skipifsourcedoesntexist

; Documentation
Source: "LICENSE.txt"; DestDir: "{app}"; Flags: ignoreversion
Source: "README.txt"; DestDir: "{app}"; Flags: ignoreversion isreadme

[Icons]
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; IconFilename: "{app}\{#MyAppExeName}"
Name: "{group}\{cm:UninstallProgram,{#MyAppName}}"; Filename: "{uninstallexe}"
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; IconFilename: "{app}\{#MyAppExeName}"; Tasks: desktopicon
Name: "{userappdata}\Microsoft\Internet Explorer\Quick Launch\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: quicklaunchicon

[Registry]
; File association (optional)
Root: HKA; Subkey: "Software\Classes\{#MyAppAssocExt}\OpenWithProgids"; ValueType: string; ValueName: "{#MyAppAssocKey}"; ValueData: ""; Flags: uninsdeletevalue
Root: HKA; Subkey: "Software\Classes\{#MyAppAssocKey}"; ValueType: string; ValueName: ""; ValueData: "{#MyAppAssocName}"; Flags: uninsdeletekey
Root: HKA; Subkey: "Software\Classes\{#MyAppAssocKey}\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: "{app}\{#MyAppExeName},0"
Root: HKA; Subkey: "Software\Classes\{#MyAppAssocKey}\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\{#MyAppExeName}"" ""%1"""

; Context menu for images
Root: HKA; Subkey: "Software\Classes\SystemFileAssociations\image\shell\DFCompressor"; ValueType: string; ValueName: ""; ValueData: "Convert with DFCompressor"; Tasks: contextmenu; Flags: uninsdeletekey
Root: HKA; Subkey: "Software\Classes\SystemFileAssociations\image\shell\DFCompressor"; ValueType: string; ValueName: "Icon"; ValueData: """{app}\{#MyAppExeName}"""; Tasks: contextmenu
Root: HKA; Subkey: "Software\Classes\SystemFileAssociations\image\shell\DFCompressor\command"; ValueType: string; ValueName: ""; ValueData: """{app}\{#MyAppExeName}"" ""%1"""; Tasks: contextmenu

; Context menu for videos
Root: HKA; Subkey: "Software\Classes\SystemFileAssociations\video\shell\DFCompressor"; ValueType: string; ValueName: ""; ValueData: "Convert with DFCompressor"; Tasks: contextmenu; Flags: uninsdeletekey
Root: HKA; Subkey: "Software\Classes\SystemFileAssociations\video\shell\DFCompressor"; ValueType: string; ValueName: "Icon"; ValueData: """{app}\{#MyAppExeName}"""; Tasks: contextmenu
Root: HKA; Subkey: "Software\Classes\SystemFileAssociations\video\shell\DFCompressor\command"; ValueType: string; ValueName: ""; ValueData: """{app}\{#MyAppExeName}"" ""%1"""; Tasks: contextmenu

; App settings location
Root: HKCU; Subkey: "Software\DFCompressor"; Flags: uninsdeletekey

[Run]
; Install VC++ Runtime if needed
Filename: "{tmp}\VC_redist.x64.exe"; Parameters: "/install /quiet /norestart"; StatusMsg: "Installing Visual C++ Runtime..."; Flags: waituntilterminated skipifsilent skipifdoesntexist

; Launch application
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent

[UninstallDelete]
Type: filesandordirs; Name: "{app}\logs"
Type: filesandordirs; Name: "{app}\cache"
Type: dirifempty; Name: "{app}"

[Code]
// Check for NVIDIA GPU
function CheckNvidiaGPU(): Boolean;
var
  ResultCode: Integer;
begin
  Result := Exec('nvidia-smi.exe', '', '', SW_HIDE, ewWaitUntilTerminated, ResultCode);
  if Result then
    Result := (ResultCode = 0);
end;

// Custom initialization
procedure InitializeWizard;
begin
  // Check for GPU
  if CheckNvidiaGPU then
  begin
    MsgBox('NVIDIA GPU detected! DFCompressor will use GPU acceleration for faster processing.', mbInformation, MB_OK);
  end
  else
  begin
    MsgBox('No NVIDIA GPU detected. DFCompressor will use CPU processing. For faster performance, consider using a system with an NVIDIA graphics card.', mbInformation, MB_OK);
  end;
end;

// Check if FFmpeg is already installed
function IsFFmpegInstalled(): Boolean;
var
  ResultCode: Integer;
begin
  Result := Exec('ffmpeg', '-version', '', SW_HIDE, ewWaitUntilTerminated, ResultCode);
end;

// Pre-install check
function InitializeSetup(): Boolean;
begin
  Result := True;
  
  // Check Windows version
  if not IsWin64 then
  begin
    MsgBox('DFCompressor requires a 64-bit version of Windows.', mbError, MB_OK);
    Result := False;
  end;
end;

// Cleanup on uninstall
procedure CurUninstallStepChanged(CurUninstallStep: TUninstallStep);
begin
  if CurUninstallStep = usPostUninstall then
  begin
    // Remove user settings if requested
    if MsgBox('Do you want to remove all DFCompressor settings and preferences?', mbConfirmation, MB_YESNO) = IDYES then
    begin
      RegDeleteKeyIncludingSubkeys(HKEY_CURRENT_USER, 'Software\DFCompressor');
      DelTree(ExpandConstant('{userappdata}\DFCompressor'), True, True, True);
    end;
  end;
end;
