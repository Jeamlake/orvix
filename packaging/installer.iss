#ifndef AppSource
  #error AppSource is required
#endif
#ifndef OutputPath
  #error OutputPath is required
#endif
#ifndef IconPath
  #error IconPath is required
#endif

[Setup]
AppId={{F7759A56-FA49-4807-9B3F-2A797128D711}
AppName=ORVIX
AppVersion=0.2.1
AppPublisher=Equipo ORVIX
AppPublisherURL=https://github.com/Jeamlake/orvix
DefaultDirName={localappdata}\Programs\ORVIX
DefaultGroupName=ORVIX
DisableProgramGroupPage=yes
PrivilegesRequired=lowest
MinVersion=10.0
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
OutputDir={#OutputPath}
OutputBaseFilename=ORVIX-Avance2-Setup-0.2.1
SetupIconFile={#IconPath}
UninstallDisplayIcon={app}\ORVIX.exe
Compression=lzma2
SolidCompression=yes
WizardStyle=modern
CloseApplications=yes
RestartApplications=no
LicenseFile=..\LICENSE

[Languages]
Name: "spanish"; MessagesFile: "compiler:Languages\Spanish.isl"

[Tasks]
Name: "desktopicon"; Description: "Crear un acceso directo en el escritorio"; GroupDescription: "Accesos directos:"; Flags: unchecked

[Files]
Source: "{#AppSource}\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{group}\ORVIX · Avance 2"; Filename: "{app}\ORVIX.exe"; WorkingDir: "{app}"
Name: "{autodesktop}\ORVIX"; Filename: "{app}\ORVIX.exe"; WorkingDir: "{app}"; Tasks: desktopicon

[Run]
Filename: "{app}\ORVIX.exe"; Description: "Abrir ORVIX"; Flags: nowait postinstall skipifsilent

; Logs and settings live outside {app} and are intentionally retained on uninstall.
