# Validación del entorno de desarrollo de ORVIX

[English](../../../en/evidence/delivery-01/environment-validation.md) | [Español](environment-validation.md)

## Equipo validado

- Windows 10 Pro x64
- Intel Core i7-9700
- 16 GB RAM
- NVIDIA RTX 2060
- GENERAL WEBCAM

## Toolchain nativo

- Visual Studio Community 2022
- MSVC x64
- C++20
- Windows SDK
- CMake
- Ninja

## Validación de Media Foundation

```text
COM_INITIALIZATION=PASS
MEDIA_FOUNDATION_STARTUP=PASS
MEDIA_FOUNDATION_CAMERA_COUNT=1
CAMERA_0=GENERAL WEBCAM
MEDIA_FOUNDATION_RUNTIME=PASS
CAMERA_PRESENT=PASS
ORVIX_NATIVE_TOOLCHAIN=PASS
```

## Incidente del linker

La validación inicial compiló, pero no pudo enlazar `MFEnumDeviceSources`.

Causa: faltaba `mf.lib` en las dependencias del objetivo.

Después de enlazar `mf`, `mfplat`, `mfreadwrite`, `mfuuid` y `ole32`, la prueba
finalizó correctamente.

## Validación de Python

Entorno aislado con Python 3.11:

- venv: PASS
- NumPy: PASS
- OpenCV: PASS
- pytest: PASS
- shared_memory: PASS

## Resultado

El equipo está validado para el desarrollo de ORVIX.
