"""SQLite persistence for sources, capture sessions, metrics, and diagnostics."""

from __future__ import annotations

import os
import sqlite3
from datetime import datetime, timezone
from pathlib import Path
from typing import Optional


def _utc_now() -> str:
    return datetime.now(timezone.utc).isoformat(timespec="milliseconds").replace("+00:00", "Z")


def _schema_path() -> Path:
    # The schema is kept in the repository's database directory so it can also be
    # applied with sqlite3 from the command line.
    package_schema = Path(__file__).with_name("schema.sql")
    if package_schema.is_file():
        return package_schema
    return Path(__file__).resolve().parents[4] / "database" / "sqlite" / "schema.sql"


class SQLiteTelemetryStore:
    """Store ORVIX telemetry in a local SQLite database.

    Example::

        with SQLiteTelemetryStore("orvix.db") as store:
            source_id = store.create_source("camera", "media_foundation", "USB Camera")
            session_id = store.start_session(source_id)
            store.record_diagnostic(session_id, "info", "CAPTURE_STARTED", "Capture started")

    Only telemetry metadata is persisted. Frame and image bytes are not accepted
    by this API or written to the database.
    """

    def __init__(self, database_path: Optional[os.PathLike] = None) -> None:
        if database_path is None:
            database_path = os.environ.get("ORVIX_DATABASE_PATH", "orvix.db")
        self.database_path = Path(database_path).expanduser()
        if str(self.database_path) != ":memory:":
            self.database_path.parent.mkdir(parents=True, exist_ok=True)

        schema_path = _schema_path()
        if not schema_path.is_file():
            raise FileNotFoundError("ORVIX SQLite schema not found: {}".format(schema_path))

        self._connection = sqlite3.connect(str(self.database_path))
        self._connection.row_factory = sqlite3.Row
        self._connection.execute("PRAGMA foreign_keys = ON")
        self._connection.executescript(schema_path.read_text(encoding="utf-8"))
        self._connection.commit()

    def close(self) -> None:
        self._connection.close()

    def __enter__(self) -> "SQLiteTelemetryStore":
        return self

    def __exit__(self, exc_type, exc_value, traceback) -> None:
        if exc_type is None:
            self._connection.commit()
        else:
            self._connection.rollback()
        self.close()

    def create_source(self, kind: str, backend: str, nombre: str) -> int:
        """Register a video source and return its ID."""
        cursor = self._connection.execute(
            "INSERT INTO source(kind, backend, nombre) VALUES (?, ?, ?)",
            (kind, backend, nombre),
        )
        self._connection.commit()
        return int(cursor.lastrowid)

    def start_session(
        self,
        source_id: int,
        *,
        started_at: Optional[str] = None,
        pixel_format: Optional[str] = None,
        width: Optional[int] = None,
        height: Optional[int] = None,
        target_fps: Optional[float] = None,
    ) -> int:
        """Start a capture session for a registered source."""
        cursor = self._connection.execute(
            """INSERT INTO session(
                   source_id, started_at, pixel_format, width, height, target_fps
               ) VALUES (?, ?, ?, ?, ?, ?)""",
            (source_id, started_at or _utc_now(), pixel_format, width, height, target_fps),
        )
        self._connection.commit()
        return int(cursor.lastrowid)

    def finish_session(
        self,
        session_id: int,
        status: str = "completed",
        *,
        ended_at: Optional[str] = None,
        error_code: Optional[str] = None,
    ) -> None:
        """Finish a running session as completed, failed, or interrupted."""
        if status not in ("completed", "failed", "interrupted"):
            raise ValueError("status must be completed, failed, or interrupted")
        cursor = self._connection.execute(
            """UPDATE session
               SET status = ?, ended_at = ?, error_code = ?
               WHERE session_id = ? AND status = 'running'""",
            (status, ended_at or _utc_now(), error_code, session_id),
        )
        if cursor.rowcount != 1:
            self._connection.rollback()
            raise ValueError("session does not exist or is already finished")
        self._connection.commit()

    def record_metric(
        self,
        session_id: int,
        *,
        recorded_at: Optional[str] = None,
        capture_fps: Optional[float] = None,
        processing_fps: Optional[float] = None,
        latency_ms: Optional[float] = None,
        skipped_frames: int = 0,
        buffer_utilization_pct: Optional[float] = None,
        buffer_overwrites: int = 0,
        detection_count: int = 0,
    ) -> int:
        """Persist a point-in-time metric sample and return its ID."""
        cursor = self._connection.execute(
            """INSERT INTO metric_sample(
                   session_id, recorded_at, capture_fps, processing_fps, latency_ms,
                   skipped_frames, buffer_utilization_pct, buffer_overwrites,
                   detection_count
               ) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)""",
            (
                session_id,
                recorded_at or _utc_now(),
                capture_fps,
                processing_fps,
                latency_ms,
                skipped_frames,
                buffer_utilization_pct,
                buffer_overwrites,
                detection_count,
            ),
        )
        self._connection.commit()
        return int(cursor.lastrowid)

    def record_diagnostic(
        self,
        session_id: int,
        severity: str,
        code: str,
        message: str,
        *,
        recorded_at: Optional[str] = None,
    ) -> int:
        """Persist a diagnostic event without storing frame contents."""
        cursor = self._connection.execute(
            """INSERT INTO diagnostic_event(
                   session_id, recorded_at, severity, code, message
               ) VALUES (?, ?, ?, ?, ?)""",
            (session_id, recorded_at or _utc_now(), severity, code, message),
        )
        self._connection.commit()
        return int(cursor.lastrowid)

    def list_sessions(self, limit: int = 100):
        """Return recent sessions with their source names, newest first."""
        if limit <= 0:
            raise ValueError("limit must be greater than zero")
        rows = self._connection.execute(
            """SELECT session.*, source.kind AS source_kind,
                      source.backend AS source_backend, source.nombre AS source_nombre
               FROM session JOIN source USING (source_id)
               ORDER BY session.started_at DESC, session.session_id DESC LIMIT ?""",
            (limit,),
        ).fetchall()
        return [dict(row) for row in rows]

    def list_metrics(self, session_id: int):
        """Return metric samples for a session in chronological order."""
        rows = self._connection.execute(
            """SELECT * FROM metric_sample
               WHERE session_id = ? ORDER BY recorded_at, sample_id""",
            (session_id,),
        ).fetchall()
        return [dict(row) for row in rows]

    def list_diagnostics(self, session_id: int):
        """Return diagnostic events for a session in chronological order."""
        rows = self._connection.execute(
            """SELECT * FROM diagnostic_event
               WHERE session_id = ? ORDER BY recorded_at, event_id""",
            (session_id,),
        ).fetchall()
        return [dict(row) for row in rows]
