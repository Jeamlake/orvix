# RF-005 — Metadatos por frame

[English](../../../en/evidence/delivery-01/rf-005-frame-metadata.md) | [Español](rf-005-frame-metadata.md)

Estado: **Validado**

Issue: **#10**

## Objetivo

Exponer suficientes metadatos de cada frame aceptado para identificar su orden,
tiempo multimedia, disposición y tamaño sin guardar la imagen.

## Campos informados

| Campo | Origen |
|---|---|
| Secuencia | Contador ORVIX incrementado por cada muestra aceptada. |
| Timestamp | Timestamp nativo en unidades de 100 nanosegundos. |
| Resolución | Tipo multimedia configurado en el lector. |
| Formato de píxel | Subtipo configurado en el lector. |
| Bytes | Longitud total del búfer contiguo de la muestra. |

## Validación con cámara física

Comando:

```bash
./build/native/Release/orvix-capture.exe capture --index 0
```

Salida representativa:

```text
Frame #1 | timestamp=... (100 ns) | resolution=1280x720 | format=NV12 | bytes=1382400
Frame #2 | timestamp=... (100 ns) | resolution=1280x720 | format=NV12 | bytes=1382400
...
Frame #120 | timestamp=... (100 ns) | resolution=1280x720 | format=NV12 | bytes=1382400

Frames captured: 120
First sequence: 1
Last sequence: 120
Captured bytes: 165888000
Sequence status: STRICTLY_INCREASING
```

Cada payload NV12 contiene los 1 382 400 bytes esperados para 1280×720 a
12 bits por píxel. La ejecución informó metadatos para las 120 muestras.

## Validación automatizada

`TC-CAP-005` utiliza un lector sintético y verifica secuencia, timestamp,
resolución 1280×720, formato NV12 y bytes del segundo frame. También verifica
la cantidad agregada y los bytes totales.

## Estado de aceptación

- secuencia por frame expuesta: **PASS**
- timestamp nativo expuesto: **PASS**
- resolución y formato expuestos: **PASS**
- cantidad de bytes expuesta: **PASS**
- frames efímeros: **PASS**
