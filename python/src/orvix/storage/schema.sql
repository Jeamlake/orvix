-- ORVIX local telemetry schema (SQLite). Frame and image payloads are never stored.
PRAGMA foreign_keys = ON;

CREATE TABLE IF NOT EXISTS schema_metadata (
    version       INTEGER PRIMARY KEY CHECK (version > 0),
    applied_at    TEXT NOT NULL DEFAULT (strftime('%Y-%m-%dT%H:%M:%fZ', 'now'))
);

-- A source is a physical camera or a synthetic/test frame producer.
CREATE TABLE IF NOT EXISTS source (
    source_id     INTEGER PRIMARY KEY,
    kind          TEXT NOT NULL CHECK (length(trim(kind)) > 0),
    backend       TEXT NOT NULL CHECK (backend IN (
                      'media_foundation', 'v4l2', 'avfoundation', 'synthetic', 'unknown'
                  )),
    nombre        TEXT NOT NULL CHECK (length(trim(nombre)) > 0)
);

CREATE TABLE IF NOT EXISTS session (
    session_id       INTEGER PRIMARY KEY,
    source_id        INTEGER NOT NULL,
    started_at       TEXT NOT NULL DEFAULT (strftime('%Y-%m-%dT%H:%M:%fZ', 'now')),
    ended_at         TEXT,
    status           TEXT NOT NULL DEFAULT 'running'
                     CHECK (status IN ('running', 'completed', 'failed', 'interrupted')),
    pixel_format     TEXT,
    width            INTEGER CHECK (width IS NULL OR width > 0),
    height           INTEGER CHECK (height IS NULL OR height > 0),
    target_fps       REAL CHECK (target_fps IS NULL OR target_fps >= 0),
    error_code       TEXT,
    FOREIGN KEY (source_id) REFERENCES source(source_id) ON DELETE RESTRICT,
    CHECK (ended_at IS NULL OR ended_at >= started_at),
    CHECK ((status = 'running' AND ended_at IS NULL) OR
           (status <> 'running' AND ended_at IS NOT NULL))
);

CREATE TABLE IF NOT EXISTS metric_sample (
    sample_id              INTEGER PRIMARY KEY,
    session_id             INTEGER NOT NULL,
    recorded_at            TEXT NOT NULL DEFAULT (strftime('%Y-%m-%dT%H:%M:%fZ', 'now')),
    capture_fps            REAL CHECK (capture_fps IS NULL OR capture_fps >= 0),
    processing_fps         REAL CHECK (processing_fps IS NULL OR processing_fps >= 0),
    latency_ms             REAL CHECK (latency_ms IS NULL OR latency_ms >= 0),
    skipped_frames         INTEGER NOT NULL DEFAULT 0 CHECK (skipped_frames >= 0),
    buffer_utilization_pct REAL CHECK (buffer_utilization_pct IS NULL OR buffer_utilization_pct BETWEEN 0 AND 100),
    buffer_overwrites      INTEGER NOT NULL DEFAULT 0 CHECK (buffer_overwrites >= 0),
    detection_count        INTEGER NOT NULL DEFAULT 0 CHECK (detection_count >= 0),
    FOREIGN KEY (session_id) REFERENCES session(session_id) ON DELETE CASCADE,
    UNIQUE (session_id, recorded_at)
);

CREATE TABLE IF NOT EXISTS diagnostic_event (
    event_id     INTEGER PRIMARY KEY,
    session_id   INTEGER NOT NULL,
    recorded_at  TEXT NOT NULL DEFAULT (strftime('%Y-%m-%dT%H:%M:%fZ', 'now')),
    severity     TEXT NOT NULL CHECK (severity IN ('debug', 'info', 'warning', 'error', 'critical')),
    code         TEXT NOT NULL CHECK (length(trim(code)) > 0),
    message      TEXT NOT NULL CHECK (length(trim(message)) > 0),
    FOREIGN KEY (session_id) REFERENCES session(session_id) ON DELETE CASCADE
);

CREATE INDEX IF NOT EXISTS idx_source_kind_backend
    ON source(kind, backend);
CREATE INDEX IF NOT EXISTS idx_session_started_at
    ON session(started_at);
CREATE INDEX IF NOT EXISTS idx_metric_sample_session_time
    ON metric_sample(session_id, recorded_at);
CREATE INDEX IF NOT EXISTS idx_diagnostic_event_session_time
    ON diagnostic_event(session_id, recorded_at);

INSERT OR IGNORE INTO schema_metadata(version) VALUES (1);
