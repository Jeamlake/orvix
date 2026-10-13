# Base de datos SQLite de ORVIX

[English](../../en/database/sqlite-schema.md) | [Español](sqlite-schema.md)

ORVIX guarda telemetría local en SQLite. El esquema y el diagrama editable están
en [`database/sqlite/schema.sql`](../../../database/sqlite/schema.sql) y
[`database/sqlite/orvix-database.drawio`](../../../database/sqlite/orvix-database.drawio).
La API Python está en `orvix.storage.SQLiteTelemetryStore`. No se guardan
imágenes ni bytes de fotogramas.

## Tablas y relaciones

| Tabla | Propósito | Relación |
|---|---|---|
| `source` | Cámara física o fuente sintética: tipo, backend y nombre. | Una fuente tiene cero o muchas sesiones. |
| `session` | Una ejecución de captura, su fuente, formato, estado y tiempos. | Pertenece a una fuente; tiene muestras y eventos. |
| `metric_sample` | FPS, latencia, fotogramas omitidos, uso del búfer y detecciones en un instante. | Pertenece a una sesión. |
| `diagnostic_event` | Severidad, código y mensaje de un diagnóstico de captura. | Pertenece a una sesión. |
| `schema_metadata` | Versión y fecha de instalación del esquema. | Independiente. |

`session.source_id` referencia `source.source_id`. Las tablas
`metric_sample` y `diagnostic_event` referencian `session.session_id`; al
eliminar una sesión se eliminan sus métricas y diagnósticos. Las restricciones
`CHECK` validan estados, severidades y métricas no negativas. Los tiempos usan
texto ISO 8601 UTC.

## Uso desde Python

La base se inicializa al crear el almacén. Por defecto, el archivo es
`orvix.db` en el directorio actual; se puede cambiar con `ORVIX_DATABASE_PATH`.

```python
from orvix.storage import SQLiteTelemetryStore

with SQLiteTelemetryStore("orvix.db") as store:
    source_id = store.create_source("camera", "media_foundation", "USB Camera")
    session_id = store.start_session(
        source_id, pixel_format="NV12", width=1280, height=720, target_fps=30
    )
    store.record_metric(session_id, capture_fps=29.8, latency_ms=8.4)
    store.record_diagnostic(session_id, "info", "CAPTURE_STARTED", "Captura iniciada")
    store.finish_session(session_id)
```

También se puede crear manualmente: `sqlite3 orvix.db < database/sqlite/schema.sql`.
SQLite forma parte de la biblioteca estándar de Python, así que no se agrega una
dependencia externa.
