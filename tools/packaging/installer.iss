; Comprueba que se proporcionó AppSource al compilar.
#ifndef AppSource
  ; Detiene la compilación con AppSource is required.
  #error AppSource is required
; Cierra la comprobación de esta opción del instalador.
#endif
; Comprueba que se proporcionó OutputPath al compilar.
#ifndef OutputPath
  ; Detiene la compilación con OutputPath is required.
  #error OutputPath is required
; Cierra la comprobación de esta opción del instalador.
#endif
; Comprueba que se proporcionó IconPath al compilar.
#ifndef IconPath
  ; Detiene la compilación con IconPath is required.
  #error IconPath is required
; Cierra la comprobación de esta opción del instalador.
#endif

; Delimita las opciones de Setup.
[Setup]
; Identifica la instalación para futuras actualizaciones con {{F7759A56-FA49-4807-9B3F-2A797128D711}.
AppId={{F7759A56-FA49-4807-9B3F-2A797128D711}
; Fija el nombre visible del producto con ORVIX.
AppName=ORVIX
; Declara la versión que instala el asistente con 1.0.0.
AppVersion=1.0.0
; Nombra al responsable de la distribución con Equipo ORVIX.
AppPublisher=Equipo ORVIX
; Enlaza al repositorio del equipo con https://github.com/Jeamlake/orvix.
AppPublisherURL=https://github.com/Jeamlake/orvix
; Instala por defecto en el perfil del usuario con {localappdata}\Programs\ORVIX.
DefaultDirName={localappdata}\Programs\ORVIX
; Nombra el grupo de accesos directos con ORVIX.
DefaultGroupName=ORVIX
; Oculta la pantalla de selección del grupo con yes.
DisableProgramGroupPage=yes
; Instala con privilegios del usuario actual con lowest.
PrivilegesRequired=lowest
; Fija la versión mínima de Windows para el instalador con 10.0.
MinVersion=10.0
; Limita las arquitecturas admitidas con x64compatible.
ArchitecturesAllowed=x64compatible
; Activa el modo de instalación de 64 bits con x64compatible.
ArchitecturesInstallIn64BitMode=x64compatible
; Selecciona la carpeta de salida con {#OutputPath}.
OutputDir={#OutputPath}
; Establece el nombre del instalador generado con ORVIX-Setup-1.0.0.
OutputBaseFilename=ORVIX-Setup-1.0.0
; Asigna el icono al asistente de instalación con {#IconPath}.
SetupIconFile={#IconPath}
; Asigna el icono de la entrada de desinstalación con {app}\ORVIX.exe.
UninstallDisplayIcon={app}\ORVIX.exe
; Selecciona el algoritmo de compresión con lzma2.
Compression=lzma2
; Comprime los archivos como un bloque con yes.
SolidCompression=yes
; Selecciona el estilo del asistente con modern.
WizardStyle=modern
; Permite cerrar la aplicación si está en uso con yes.
CloseApplications=yes
; Decide si se relanza la aplicación al terminar con no.
RestartApplications=no
; Muestra la licencia del proyecto durante la instalación con ..\..\LICENSE.
LicenseFile=..\..\LICENSE

; Delimita las opciones de Languages.
[Languages]
; Carga los mensajes en español para el instalador.
Name: "spanish"; MessagesFile: "compiler:Languages\Spanish.isl"

; Delimita las opciones de Tasks.
[Tasks]
; Declara la tarea opcional del acceso directo de escritorio.
Name: "desktopicon"; Description: "Crear un acceso directo en el escritorio"; GroupDescription: "Accesos directos:"; Flags: unchecked

; Delimita las opciones de Files.
[Files]
; Copia la distribución portátil y sus subcarpetas a la ruta instalada.
Source: "{#AppSource}\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs

; Delimita las opciones de Icons.
[Icons]
; Crea el acceso directo del menú de inicio.
Name: "{group}\ORVIX"; Filename: "{app}\ORVIX.exe"; WorkingDir: "{app}"
; Crea el acceso directo de escritorio cuando se seleccionó la tarea.
Name: "{autodesktop}\ORVIX"; Filename: "{app}\ORVIX.exe"; WorkingDir: "{app}"; Tasks: desktopicon

; Delimita las opciones de Run.
[Run]
; Ofrece abrir ORVIX al completar una instalación interactiva.
Filename: "{app}\ORVIX.exe"; Description: "Abrir ORVIX"; Flags: nowait postinstall skipifsilent

; Logs and settings live outside {app} and are intentionally retained on uninstall.
