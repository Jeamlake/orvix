# RF-015 — FPS efectivos de captura

[English](../../../en/evidence/delivery-01/rf-015-capture-fps.md) | [Español](rf-015-capture-fps.md)

Estado: **Validado**

Issue: **#11**

## Objetivo

Medir la frecuencia efectiva de los frames aceptados independientemente de los
FPS solicitados durante la negociación.

## Cálculo

Para una captura con al menos dos muestras:

```text
elapsed_seconds = (last_timestamp_100ns - first_timestamp_100ns) / 10,000,000
capture_fps = (captured_frames - 1) / elapsed_seconds
```

El numerador utiliza intervalos: una captura de 120 frames mide 119 intervalos.
Los timestamps están expresados en unidades de 100 nanosegundos.

## Validación con cámara física

El equipo de desarrollo produjo:

```text
Negotiated resolution: 1280x720
Negotiated pixel format: NV12
Negotiated FPS: 30.00
Frames captured: 120
Capture duration: 6.112 s
Measured capture FPS: 19.47
Capture status: CAPTURE_COMPLETED
```

El tipo multimedia se configuró a 30 FPS y la velocidad efectiva observada fue
19.47 FPS. ORVIX informa ambos valores por separado.

## Validación automatizada

`TC-MET-001` proporciona tres frames sintéticos separados por 333 333 unidades
y verifica un resultado de 30 FPS con una pequeña tolerancia de punto flotante.

## Estado de aceptación

- tiempo multimedia calculado: **PASS**
- FPS efectivos calculados: **PASS**
- FPS negociados y medidos separados: **PASS**
- medición física completada: **PASS**
