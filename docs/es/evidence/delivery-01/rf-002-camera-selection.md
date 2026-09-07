# RF-002 — Selección de cámara

[English](../../../en/evidence/delivery-01/rf-002-camera-selection.md) | [Español](rf-002-camera-selection.md)

Estado: **Validado**

Issue: **#3**

## Objetivo

Seleccionar un dispositivo de captura de la lista enumerada por ORVIX Capture.

## Arquitectura

La selección es independiente de Media Foundation:

```text
Lista CameraDevice -> CameraSelector -> CameraDevice seleccionado
```

RF-002 no abre el dispositivo; la apertura pertenece a RF-003.

## CLI

```text
orvix-capture select --index <N>
```

## Validación en el equipo de desarrollo

```text
ORVIX Capture Core 0.1.0

Selected video capture device:

Index: 0
Name: GENERAL WEBCAM
Backend: Windows Media Foundation
Symbolic link: [REDACTED_LOCAL_DEVICE_IDENTIFIER]

Selection status: READY_FOR_OPEN
```

## Validación de selección inválida

- índice inválido rechazado: **PASS**
- código de diagnóstico: `ORV-CAP-404`
- cierre inesperado del proceso: **NO**

## Validación automatizada

- selección válida con `CameraSelector`: **PASS**
- selección inválida con `CameraSelector`: **PASS**
- tratamiento de lista vacía: **PASS**
- CTest: **PASS**
- CI: **PASS**

## Estado de aceptación

- el índice válido selecciona la cámara correcta: **PASS**
- el índice inválido se controla con seguridad: **PASS**
- la selección es independiente de Media Foundation: **PASS**
- `GENERAL WEBCAM` seleccionada en el equipo: **PASS**
- trazabilidad actualizada: **PASS**

## Validación con GitHub Actions

- estado: completed
- conclusión: success
- commit: dfacfc64865078a1af4a909db60a577d597dede6
- ejecución: https://github.com/Jeamlake/orvix/actions/runs/33390474864
