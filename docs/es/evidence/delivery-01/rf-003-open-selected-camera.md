# RF-003 — Apertura de la cámara seleccionada

[English](../../../en/evidence/delivery-01/rf-003-open-selected-camera.md) | [Español](rf-003-open-selected-camera.md)

Estado: **Validado**

Issue: **#5**

## Objetivo

Abrir desde C++ nativo el dispositivo seleccionado mediante Windows Media
Foundation.

## Arquitectura

```text
Lista CameraDevice
      |
      v
CameraSelector
      |
      v
MediaFoundationCamera
      |
      v
MFCreateDeviceSource
      |
      v
IMFMediaSource (ACTIVE)
```

## CLI

```text
orvix-capture open --index <N>
```

## Implementación

La funcionalidad incorpora:

- guardas reutilizables para COM y Media Foundation;
- `MediaFoundationCamera`, propietaria de `IMFMediaSource`;
- apertura mediante el enlace simbólico de la cámara seleccionada;
- validación con `IMFMediaSource::GetCharacteristics`;
- `Shutdown` y liberación COM deterministas mediante RAII;
- tratamiento controlado de índices inválidos, descriptores incompletos y
  dispositivos no disponibles;
- pruebas nativas independientes del hardware.

## Validación en el equipo de desarrollo

```text
ORVIX Capture Core 0.1.0

Opened video capture device:

Index: 0
Name: GENERAL WEBCAM
Backend: Windows Media Foundation

Open status: OPEN
Media source: ACTIVE
```

## Validación automatizada

- estado inicial cerrado: **PASS**
- cierre repetido seguro: **PASS**
- enlace simbólico vacío rechazado: **PASS**
- dispositivo no disponible rechazado sin crash: **PASS**
- índice inválido del CLI rechazado: **PASS**
- pruebas RF-001 y RF-002 conservadas: **PASS**

## Estado de aceptación

- apertura mediante Media Foundation: **PASS**
- fuente multimedia activa: **PASS**
- propiedad de la cámara en C++: **PASS**
- liberación determinista: **PASS**
- compilación local: **PASS**
- pruebas independientes del hardware: **PASS**
- trazabilidad actualizada: **PASS**

## Validación con GitHub Actions

- estado: completed
- conclusión: success
- commit: 5555af65763259d8ef6d0008079117b4b45722a8
- ejecución: https://github.com/Jeamlake/orvix/actions/runs/34081939276

El job nativo realiza una compilación MSVC limpia y ejecuta las pruebas que no
requieren hardware. La apertura física se validó en el equipo de desarrollo.

## Límite del alcance

RF-003 prueba la apertura del dispositivo. La negociación del tipo multimedia
y la captura continua pertenecen a los siguientes incrementos.

## Privacidad

El enlace simbólico específico del equipo no se guarda en la evidencia.
