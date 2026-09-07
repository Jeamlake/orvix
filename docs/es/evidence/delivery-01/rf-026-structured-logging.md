# RF-026 — Logging estructurado de captura

[English](../../../en/evidence/delivery-01/rf-026-structured-logging.md) | [Español](rf-026-structured-logging.md)

Estado: **Validado**

Issue: **#13**

## Objetivo

Guardar diagnósticos legibles del ciclo de vida y de los fallos de captura sin
escribir el contenido de los frames en disco.

## Destino y esquema

El CLI agrega registros a `logs/orvix-capture.log` y muestra esa ruta. Cada
registro contiene timestamp UTC ISO-8601, severidad, código estable, evento y
mensaje escapado.

```text
timestamp=2026-09-07T...Z level=INFO code=ORV-CAP-200 event=camera_opened message="..."
timestamp=2026-09-07T...Z level=INFO code=ORV-CAP-211 event=camera_format_configured message="resolution=1280x720 pixel_format=NV12 fps=30.00 native_type_index=2"
timestamp=2026-09-07T...Z level=INFO code=ORV-CAP-220 event=capture_started message="requested_frames=120"
timestamp=2026-09-07T...Z level=INFO code=ORV-CAP-221 event=capture_completed message="captured_frames=120 first_sequence=1 last_sequence=120 total_bytes=165888000 duration_seconds=6.112 measured_fps=19.47"
timestamp=2026-09-07T...Z level=INFO code=ORV-CAP-202 event=camera_closed message="Camera resources released after capture."
```

La validación de índice inválido también produjo un registro `ERROR` con código
`ORV-CAP-404` y evento `invalid_camera_index`.

## Validación automatizada

`TC-LOG-001` escribe eventos informativos y de error en un archivo temporal,
verifica los campos y el escape de comillas, y elimina el archivo.

## Estado de aceptación

- directorio de log creado automáticamente: **PASS**
- registros agregados y vaciados: **PASS**
- eventos de ciclo de vida y fallos registrados: **PASS**
- campos de diagnóstico estables: **PASS**
- mensajes multilínea y con comillas escapados: **PASS**
- imágenes de frames omitidas: **PASS**

El catálogo completo se mantiene en
[diagnósticos de captura](../../observability/capture-diagnostics.md).
