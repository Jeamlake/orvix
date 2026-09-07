# Platform Compatibility

ORVIX uses the operating system's native camera API through one common C++20
interface. The command line is the same on every supported platform.

## Support matrix

| Platform | Backend | Compiler validation | Physical-camera validation |
|---|---|---|---|
| Windows 10/11 x64 | Media Foundation | GitHub Actions + local MSVC | PASS on `GENERAL WEBCAM` |
| Windows 7 SP1 x64 | Media Foundation legacy target | Win32 6.1 source target + Python 3.8 CI | Requires validation on a Windows 7 host |
| Ubuntu Linux x64 | V4L2 | GitHub Actions GCC | Requires a Linux host with `/dev/video*` |
| macOS | AVFoundation | GitHub Actions Apple Clang | Requires a Mac with camera permission |

CI proves that each native implementation compiles, links, selects the correct
factory and handles an empty hardware environment. Hosted runners do not expose
physical webcams, so a real camera must be used for the final hardware check on
each target family.

## Windows 10 and 11

Prerequisites:

- Visual Studio with Desktop development with C++;
- Windows SDK;
- CMake 3.20 or newer;
- Python 3.11 recommended.

```powershell
.\scripts\setup.ps1
.\scripts\build.ps1
ctest --test-dir build -C Release --output-on-failure
.\build\native\Release\orvix-capture.exe devices
.\build\native\Release\orvix-capture.exe capture --index 0
```

Allow desktop applications to use the camera in Windows privacy settings.

## Windows 7 SP1 legacy target

Use Visual Studio 2019 with the v142 C++ toolset, an SDK that can target Windows
7, a CMake release supported by the host and Python 3.8. The default CMake
option `ORVIX_WINDOWS_7_COMPAT=ON` restricts Win32 declarations to version 6.1
and links the MSVC runtime statically.

```powershell
.\scripts\setup.ps1 -PythonVersion 3.8
cmake -S . -B build -G "Visual Studio 16 2019" -A x64 -DORVIX_WINDOWS_7_COMPAT=ON
cmake --build build --config Release --parallel
ctest --test-dir build -C Release --output-on-failure
```

Windows 7 N editions also require the applicable Media Feature Pack. A release
claimed for Windows 7 must be executed on an actual Windows 7 SP1 installation;
current GitHub-hosted runners cannot provide that runtime test.

## Linux

Install a C++ compiler, CMake, Python and Linux video headers. On Ubuntu:

```bash
sudo apt-get update
sudo apt-get install build-essential cmake python3-venv linux-libc-dev
./scripts/setup.sh
./scripts/build.sh
./scripts/test.sh
./build/native/orvix-capture devices
./build/native/orvix-capture capture --index 0
```

The user must have read/write access to the selected `/dev/videoN` device. On
many distributions this is provided through the `video` group.

## macOS

Install Xcode Command Line Tools, CMake and Python 3:

```bash
xcode-select --install
./scripts/setup.sh
./scripts/build.sh
./scripts/test.sh
./build/native/orvix-capture devices
./build/native/orvix-capture capture --index 0
```

The executable embeds `NSCameraUsageDescription`. On first capture, macOS asks
for camera permission. If access was previously denied, enable the terminal or
host application under System Settings, Privacy & Security, Camera.

## Common verification

A successful physical run reports the selected platform backend, 120 captured
frames, a sequence from 1 to 120, monotonic timestamps, measured FPS and
`Capture status: CAPTURE_COMPLETED`. Frames remain ephemeral on every backend.
