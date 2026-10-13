# ORVIX SQLite database

[English](sqlite-schema.md) | [Español](../../es/database/sqlite-schema.md)

ORVIX stores telemetry locally in SQLite. The schema and editable diagram are
in [`database/sqlite/schema.sql`](../../../database/sqlite/schema.sql) and
[`database/sqlite/orvix-database.drawio`](../../../database/sqlite/orvix-database.drawio).
The Python API is `orvix.storage.SQLiteTelemetryStore`. Images and frame bytes
are never stored.

## Tables and relationships

| Table | Purpose | Relationship |
|---|---|---|
| `source` | Physical camera or synthetic source: kind, backend, and name. | One source has zero or many sessions. |
| `session` | One capture run, its source, format, status, and timestamps. | Belongs to a source; has samples and events. |
| `metric_sample` | Point-in-time FPS, latency, skipped frames, buffer use, and detections. | Belongs to a session. |
| `diagnostic_event` | Severity, code, and message for a capture diagnostic. | Belongs to a session. |
| `schema_metadata` | Installed schema version and timestamp. | Standalone. |

`session.source_id` references `source.source_id`. `metric_sample` and
`diagnostic_event` reference `session.session_id`; deleting a session deletes
its metrics and diagnostics. `CHECK` constraints validate statuses, severities,
and non-negative metrics. Timestamps use ISO 8601 UTC text.

## Use from Python

The database initializes when the store is created. By default, it creates
`orvix.db` in the current directory; set `ORVIX_DATABASE_PATH` to change the
location.

```python
from orvix.storage import SQLiteTelemetryStore

with SQLiteTelemetryStore("orvix.db") as store:
    source_id = store.create_source("camera", "media_foundation", "USB Camera")
    session_id = store.start_session(
        source_id, pixel_format="NV12", width=1280, height=720, target_fps=30
    )
    store.record_metric(session_id, capture_fps=29.8, latency_ms=8.4)
    store.record_diagnostic(session_id, "info", "CAPTURE_STARTED", "Capture started")
    store.finish_session(session_id)
```

It can also be created manually: `sqlite3 orvix.db < database/sqlite/schema.sql`.
SQLite is part of Python's standard library, so no external dependency is added.
