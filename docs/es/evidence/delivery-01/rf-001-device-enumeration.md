# RF-001 — Enumeración de dispositivos con Media Foundation

[English](../../../en/evidence/delivery-01/rf-001-device-enumeration.md) | [Español](rf-001-device-enumeration.md)

Estado: **Validado**

Issue: **#1**

## Objetivo

Enumerar dispositivos de captura de video desde la capa C++ nativa mediante
Windows Media Foundation.

## Arquitectura

```text
orvix-capture
      |
      v
DeviceEnumerator
      |
      v
MediaFoundationDeviceEnumerator
      |
      v
Windows Media Foundation
      |
      v
Dispositivos de cámara físicos
```

## Implementación

La funcionalidad incorpora:

- modelo de dominio `CameraDevice`;
- abstracción `DeviceEnumerator`;
- `MediaFoundationDeviceEnumerator`;
- administración RAII para COM y Media Foundation;
- tratamiento de errores basado en HRESULT;
- comando `orvix-capture devices`;
- validación nativa independiente del hardware.

## Validación local

### CMake

```text
configure = PASS
build     = PASS
```

### Pruebas nativas

```text
CTest = PASS
```

### Ejecución en el equipo de desarrollo

```text
ORVIX Capture Core 0.1.0

Backend: Windows Media Foundation

Available video capture devices:

[0] GENERAL WEBCAM
    Backend: Windows Media Foundation
    Symbolic link: [REDACTED_LOCAL_DEVICE_IDENTIFIER]

Device count: 1
```

## Estado de aceptación

- C++ enumera mediante Media Foundation: **PASS**
- `GENERAL WEBCAM` detectada: **PASS**
- modelo de dispositivo dedicado: **PASS**
- lógica de Media Foundation aislada de `main.cpp`: **PASS**
- caso sin cámaras controlado: **IMPLEMENTADO**
- compilación local: **PASS**
- pruebas independientes del hardware: **PASS**
- CI: **PASS**
- trazabilidad actualizada: **PASS**

## Privacidad

La evidencia omite intencionalmente el enlace simbólico específico del equipo.

## Validación con GitHub Actions

- estado: completed
- conclusión: success
- commit: af4e298e4ddca13e5f3d0fb86a2213ed386e8c4b
- ejecución: https://github.com/Jeamlake/orvix/actions/runs/33388027430

El CI valida la compilación y las pruebas independientes del hardware. La
enumeración real se validó por separado en el equipo de desarrollo.
