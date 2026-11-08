"""Local session history. Only metrics and diagnostic text are persisted."""

import sqlite3
from datetime import datetime, timezone

from orvix import __version__

_SCHEMA = """
CREATE TABLE IF NOT EXISTS source (
    source_id TEXT PRIMARY KEY,
    kind TEXT NOT NULL,
    display_name TEXT NOT NULL,
    backend TEXT NOT NULL
);
CREATE TABLE IF NOT EXISTS session (
    session_id TEXT PRIMARY KEY,
    source_id TEXT NOT NULL REFERENCES source(source_id),
    started_at_utc TEXT NOT NULL,
    ended_at_utc TEXT,
    duration_s REAL,
    initial_mode TEXT NOT NULL,
    profile TEXT NOT NULL,
    slot_count INTEGER NOT NULL,
    sample_interval_s INTEGER NOT NULL,
    motion_area_percent REAL NOT NULL,
    width INTEGER,
    height INTEGER,
    pixel_format TEXT,
    nominal_fps REAL,
    state TEXT NOT NULL,
    version TEXT NOT NULL
);
CREATE TABLE IF NOT EXISTS metric_sample (
    session_id TEXT NOT NULL REFERENCES session(session_id),
    sequence INTEGER NOT NULL,
    elapsed_s REAL NOT NULL,
    mode TEXT NOT NULL,
    capture_fps REAL NOT NULL,
    processing_fps REAL NOT NULL,
    latency_ms REAL NOT NULL,
    skipped INTEGER NOT NULL,
    overwrites INTEGER NOT NULL,
    buffer_percent REAL NOT NULL,
    detections_min INTEGER NOT NULL,
    detections_total INTEGER NOT NULL,
    PRIMARY KEY (session_id, sequence)
);
CREATE TABLE IF NOT EXISTS diagnostic_event (
    event_id INTEGER PRIMARY KEY,
    session_id TEXT NOT NULL REFERENCES session(session_id),
    elapsed_s REAL NOT NULL,
    level TEXT NOT NULL,
    message TEXT NOT NULL
);
CREATE INDEX IF NOT EXISTS session_started ON session(started_at_utc);
CREATE INDEX IF NOT EXISTS diagnostic_session ON diagnostic_event(session_id, event_id);
PRAGMA user_version = 1;
"""


def utc_now():
    return datetime.now(timezone.utc).isoformat(timespec="milliseconds")


class History:
    def __init__(self, path):
        self.db = sqlite3.connect(str(path), timeout=0.1)
        self.db.row_factory = sqlite3.Row
        try:
            self.db.execute("PRAGMA foreign_keys = ON")
            version = self.db.execute("PRAGMA user_version").fetchone()[0]
            if version == 0:
                self.db.executescript("BEGIN IMMEDIATE;" + _SCHEMA + "COMMIT;")
            elif version != 1:
                raise sqlite3.DatabaseError("Versión de historial incompatible.")
        except Exception:
            self.db.close()
            raise

    def close(self):
        self.db.close()

    def begin(self, session_id, kind, source, backend, mode, profile, slots, interval, motion_area):
        source_id = "|".join((kind, backend, source))
        with self.db:
            self.db.execute(
                "INSERT OR IGNORE INTO source VALUES (?, ?, ?, ?)",
                (source_id, kind, source, backend),
            )
            self.db.execute(
                """INSERT INTO session
                   (session_id, source_id, started_at_utc, initial_mode, profile,
                    slot_count, sample_interval_s, motion_area_percent, state, version)
                   VALUES (?, ?, ?, ?, ?, ?, ?, ?, 'created', ?)""",
                (session_id, source_id, utc_now(), mode, profile, slots, interval, motion_area, __version__),
            )

    def connected(self, session_id, header):
        with self.db:
            self.db.execute(
                """UPDATE session SET width=?, height=?, pixel_format=?, nominal_fps=?,
                   state='running' WHERE session_id=?""",
                (header.width, header.height, header.pixel_format,
                 header.fps_numerator / max(1, header.fps_denominator), session_id),
            )

    def sample(self, session_id, elapsed, data):
        with self.db:
            self.db.execute(
                "INSERT OR IGNORE INTO metric_sample VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)",
                (session_id, data["sequence"], elapsed, data["mode"], data["capture_fps"],
                 data["consumer_fps"], data["latency_ms"], data["skipped"], data["overwrites"],
                 data["buffer"], data["detections_min"], data["detections_total"]),
            )

    def event(self, session_id, elapsed, level, message):
        with self.db:
            self.db.execute(
                "INSERT INTO diagnostic_event (session_id, elapsed_s, level, message) VALUES (?, ?, ?, ?)",
                (session_id, elapsed, level, message[:1000]),
            )

    def finish(self, session_id, duration, state):
        with self.db:
            self.db.execute(
                "UPDATE session SET ended_at_utc=?, duration_s=?, state=? WHERE session_id=?",
                (utc_now(), duration, state, session_id),
            )

    def sessions(self):
        return self.db.execute(
            """SELECT session.*, source.display_name, source.backend
               FROM session JOIN source USING(source_id)
               ORDER BY started_at_utc DESC LIMIT 100"""
        ).fetchall()

    def summary(self, session_id):
        return self.db.execute(
            """SELECT COUNT(*) AS samples, AVG(capture_fps) AS capture,
                      AVG(processing_fps) AS processing, AVG(latency_ms) AS latency,
                      MAX(skipped) AS skipped, MAX(overwrites) AS overwrites,
                      MAX(detections_total) AS detections
               FROM metric_sample WHERE session_id=?""", (session_id,),
        ).fetchone()

    def samples(self, session_id):
        return list(reversed(self.db.execute(
            """SELECT * FROM metric_sample WHERE session_id=?
               ORDER BY sequence DESC LIMIT 600""", (session_id,),
        ).fetchall()))

    def events(self, session_id):
        return list(reversed(self.db.execute(
            """SELECT elapsed_s, level, message FROM diagnostic_event WHERE session_id=?
               ORDER BY event_id DESC LIMIT 50""", (session_id,),
        ).fetchall()))
