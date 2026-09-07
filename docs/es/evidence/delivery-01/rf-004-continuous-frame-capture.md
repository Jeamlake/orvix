# RF-004 — Captura continua de frames

[English](../../../en/evidence/delivery-01/rf-004-continuous-frame-capture.md) | [Español](rf-004-continuous-frame-capture.md)

Estado: **Validado**

Issue: **#8**

## Objetivo

Adquirir muestras reales consecutivas de la cámara seleccionada y demostrar
que la secuencia de ORVIX aumenta.

## Arquitectura

```text
Cámara física
      |
      v
IMFMediaSource
      |
      v
IMFSourceReader
      |
      v
Bucle ReadSample
      |
      v
Secuencia + resumen de captura
```

## CLI

```text
orvix-capture capture --index <N>
```

La sesión inicial solicita 120 frames y finaliza después de esa ejecución
acotada.

## Validación en el equipo de desarrollo

```text
Device: GENERAL WEBCAM
Backend: Windows Media Foundation
Target resolution: 1280x720
Target FPS: 30
Negotiated resolution: 1280x720
Negotiated pixel format: NV12
Negotiated FPS: 30.00
Negotiated native type index: 2

Frames requested: 120
Frames captured: 120
First sequence: 1
Last sequence: 120
Captured bytes: 165888000
Empty reads: 1
Sequence status: STRICTLY_INCREASING
Capture status: CAPTURE_COMPLETED
```

Los bytes corresponden a 120 frames NV12 de 1280×720: 1 382 400 bytes por
frame.

## Tratamiento controlado de fallos

La ruta rechaza:

- una solicitud de cero frames;
- un índice inválido;
- captura antes de abrir la cámara;
- captura antes de configurar el formato;
- errores del lector o final prematuro del stream;
- cambio del tipo multimedia durante la captura;
- exceso de lecturas vacías;
- overflow del contador de bytes.

## Validación automatizada

- el lector sintético tolera una lectura vacía: **PASS**
- se alcanza la cantidad solicitada: **PASS**
- la secuencia comienza en 1 y termina en 120: **PASS**
- timestamps y bytes se resumen: **PASS**
- solicitud de cero frames rechazada: **PASS**
- índice inválido rechazado: **PASS**
- pruebas anteriores conservadas: **PASS**

## Estado de aceptación

- muestras consecutivas de cámara física: **PASS**
- secuencia creciente: **PASS**
- formato negociado activo: **PASS**
- captura controlada completa: **PASS**
- compilación MSVC local: **PASS**
- pruebas independientes del hardware: **PASS**
- trazabilidad actualizada: **PASS**

## Validación con GitHub Actions

- estado: completed
- conclusión: success
- commit: e4455f3b07ea7bebfce305a82eb0eb77b8123d3b
- ejecución: https://github.com/Jeamlake/orvix/actions/runs/34082967234

El lector sintético valida el bucle continuo sin cámara; la adquisición física
se validó por separado en el equipo de desarrollo.

## Límite del alcance

RF-004 cuenta y valida muestras sin guardar imágenes. Los metadatos pertenecen
a RF-005 y la medición de FPS efectivos pertenece a RF-015.
