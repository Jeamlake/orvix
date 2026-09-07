# Diagnósticos de captura nativa

[English](../../en/observability/capture-diagnostics.md) | [Español](capture-diagnostics.md)

ORVIX informa los fallos esperados mediante un código estable, un mensaje
legible, un evento estructurado y un código de salida distinto de cero. El
archivo `logs/orvix-capture.log` se ubica respecto al directorio de ejecución.

## Formato del log

Cada registro ocupa una línea:

```text
timestamp=<UTC ISO-8601> level=<NIVEL> code=<CÓDIGO> event=<EVENTO> message="<DETALLES>"
```

Los mensajes escapan comillas y barras inversas y sustituyen saltos de línea
por espacios. El logger agrega y vacía cada registro inmediatamente.

## Catálogo de diagnósticos

| Código | Evento | Salida | Significado |
|---|---|---:|---|
| ORV-CAP-100 | `camera_absent` | 65 | No hay una cámara disponible para el comando. |
| ORV-CAP-404 | `invalid_camera_index` | 66 | El índice está fuera de la lista enumerada. |
| ORV-CAP-501 | `camera_open_failed` | 71 | No se pudo activar la cámara seleccionada. |
| ORV-CAP-502 | `format_negotiation_failed` | 72 | No se pudo configurar un tipo de medio utilizable. |
| ORV-CAP-503 | `camera_disconnected` | 74 | El dispositivo fue retirado o invalidado. |
| ORV-CAP-503 | `camera_stream_error` | 74 | El lector informó un error de stream. |
| ORV-CAP-503 | `camera_stream_ended` | 74 | El stream terminó antes de completar la captura. |
| ORV-CAP-503 | `camera_stream_stalled` | 74 | Se superó el límite de lecturas sin frame. |
| ORV-CAP-504 | `frame_read_failed` | 75 | Falló una lectura por otra causa de la API nativa. |
| ORV-CAP-505 | `camera_format_changed` | 75 | El tipo de medio cambió durante la captura. |
| ORV-CAP-400 | `invalid_argument` | 64 | Un argumento del CLI está mal formado. |
| ORV-CAP-400 | `invalid_command` | 64 | La forma del comando no es compatible. |
| ORV-CAP-500 | `native_api_failure` | 70 | Fallo no clasificado de la API nativa. |
| ORV-CAP-599 | `unexpected_failure` | 70 | Excepción inesperada que escapó del comando. |

`orvix-capture devices` considera una lista vacía como enumeración correcta,
con advertencia y conteo cero. Los comandos que requieren cámara devuelven
`ORV-CAP-100`.

## Eventos del ciclo de vida

Las operaciones correctas utilizan códigos informativos entre `ORV-CAP-101` y
`ORV-CAP-221` para enumeración, selección, apertura, descubrimiento y
configuración de formato, inicio, finalización y liberación de recursos.

El registro de finalización incluye cantidad de frames, primera y última
secuencia, bytes totales, tiempo multimedia transcurrido y FPS medidos. Nunca se
escriben imágenes en el log.
