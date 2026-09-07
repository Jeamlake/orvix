# ORVIX Development Environment Validation

## Validated Workstation

- Windows 10 Pro x64
- Intel Core i7-9700
- 16 GB RAM
- NVIDIA RTX 2060
- GENERAL WEBCAM

## Native Toolchain

- Visual Studio Community 2022
- MSVC x64
- C++20
- Windows SDK
- CMake
- Ninja

## Media Foundation Validation

```text
COM_INITIALIZATION=PASS
MEDIA_FOUNDATION_STARTUP=PASS
MEDIA_FOUNDATION_CAMERA_COUNT=1
CAMERA_0=GENERAL WEBCAM
MEDIA_FOUNDATION_RUNTIME=PASS
CAMERA_PRESENT=PASS
ORVIX_NATIVE_TOOLCHAIN=PASS
```

## Linker Incident

The initial validation compiled but failed to link `MFEnumDeviceSources`.

Root cause: `mf.lib` was missing from target dependencies.

After linking `mf`, `mfplat`, `mfreadwrite`, `mfuuid` and `ole32`, the test passed.

## Python Validation

Python 3.11 isolated environment:

- venv: PASS
- NumPy: PASS
- OpenCV: PASS
- pytest: PASS
- shared_memory: PASS

## Result

The workstation is validated for ORVIX development.