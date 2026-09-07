# Compatibilidad de plataformas

[English](../../en/platform/compatibility.md) | [Español](compatibility.md)

ORVIX utiliza la API nativa de cámara de cada sistema operativo mediante una
interfaz común en C++20. El CLI conserva los mismos comandos en todas las
plataformas compatibles.

## Matriz de compatibilidad

| Plataforma | Backend | Validación de compilación | Validación con cámara física |
|---|---|---|---|
| Windows 10/11 x64 | Media Foundation | GitHub Actions + MSVC local | PASS con `GENERAL WEBCAM` |
| Windows 7 SP1 x64 | Objetivo heredado de Media Foundation | Código objetivo Win32 6.1 + CI con Python 3.8 | Requiere un equipo con Windows 7 |
| Ubuntu Linux x64 | V4L2 | GitHub Actions con GCC | Requiere Linux con `/dev/video*` |
| macOS | AVFoundation | GitHub Actions con Apple Clang | Requiere una Mac con permiso de cámara |

El CI demuestra que cada implementación nativa compila, enlaza, selecciona la
factoría correcta y maneja un entorno sin hardware. Los runners hospedados no
incluyen webcams físicas, por lo que la comprobación final de cada familia
requiere una cámara real.

## Windows 10 y 11

Requisitos:

- Visual Studio con **Desarrollo para el escritorio con C++**;
- Windows SDK;
- CMake 3.20 o posterior;
- Python 3.11 recomendado.

```powershell
.\scripts\setup.ps1
.\scripts\build.ps1
.\scripts\test.ps1
.\build\native\Release\orvix-capture.exe devices
.\build\native\Release\orvix-capture.exe capture --index 0
```

Windows debe permitir que las aplicaciones de escritorio accedan a la cámara.

### Git Bash en Windows

Los scripts portables detectan Git Bash (`MINGW`, `MSYS` o `CYGWIN`) y utilizan
la estructura del entorno virtual de Windows y el generador de Visual Studio:

```bash
./scripts/setup.sh
./scripts/build.sh
./scripts/test.sh
./build/native/Release/orvix-capture.exe devices
./build/native/Release/orvix-capture.exe capture --index 0
```

En Git Bash se utilizan barras `/`. Los scripts `.ps1` siguen disponibles para
PowerShell.

## Objetivo heredado Windows 7 SP1

Utiliza Visual Studio 2019 con el toolset C++ v142, un SDK capaz de apuntar a
Windows 7, una versión de CMake compatible con el equipo y Python 3.8. La opción
predeterminada `ORVIX_WINDOWS_7_COMPAT=ON` limita las declaraciones Win32 a la
versión 6.1 y enlaza estáticamente el runtime de MSVC.

```powershell
.\scripts\setup.ps1 -PythonVersion 3.8
cmake -S . -B build -G "Visual Studio 16 2019" -A x64 -DORVIX_WINDOWS_7_COMPAT=ON
cmake --build build --config Release --parallel
ctest --test-dir build -C Release --output-on-failure
```

Las ediciones Windows 7 N también requieren el Media Feature Pack aplicable.
Una versión declarada compatible debe ejecutarse en una instalación real de
Windows 7 SP1; los runners actuales de GitHub no permiten esa prueba.

## Linux

Instala un compilador C++, CMake, Python y los headers de video de Linux. En
Ubuntu:

```bash
sudo apt-get update
sudo apt-get install build-essential cmake python3-venv linux-libc-dev
./scripts/setup.sh
./scripts/build.sh
./scripts/test.sh
./build/native/orvix-capture devices
./build/native/orvix-capture capture --index 0
```

El usuario necesita permisos de lectura y escritura sobre `/dev/videoN`. En
muchas distribuciones se obtienen mediante el grupo `video`.

## macOS

Instala Xcode Command Line Tools, CMake y Python 3:

```bash
xcode-select --install
./scripts/setup.sh
./scripts/build.sh
./scripts/test.sh
./build/native/orvix-capture devices
./build/native/orvix-capture capture --index 0
```

El ejecutable incluye `NSCameraUsageDescription`. macOS solicitará permiso la
primera vez. Si se rechazó previamente, habilita la terminal o aplicación en
**Configuración del Sistema > Privacidad y seguridad > Cámara**.

## Verificación común

Una ejecución física correcta informa el backend seleccionado, 120 frames, una
secuencia de 1 a 120, timestamps crecientes, los FPS medidos y
`Capture status: CAPTURE_COMPLETED`. Los frames permanecen efímeros en todos
los backends.
