"""Local session history. Only metrics and diagnostic text are persisted."""

# Importa sqlite3.
import sqlite3
# Importa datetime, timezone desde datetime.
from datetime import datetime, timezone

# Importa __version__ desde orvix.
from orvix import __version__

# Define las tablas SQLite de fuentes, sesiones, muestras y eventos, sus claves, índices y versión.
# Source identifica cada fuente y conserva su tipo, nombre y backend.
# Session referencia a source y registra tiempos, perfil, formato y estado.
# Metric_sample referencia a session y distingue las muestras por sesión y secuencia.
# Diagnostic_event vincula cada mensaje y su nivel con la sesión correspondiente.
# Los índices aceleran el listado de sesiones y la consulta de sus eventos.
# User_version identifica el formato que debe reconocer el historial.
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


# Define utc_now sin parámetros de llamada.
def utc_now():
    # Devuelve datetime.now(timezone.utc).isoformat(timespec='milliseconds').
    return datetime.now(timezone.utc).isoformat(timespec="milliseconds")


# Declara el tipo History para agrupar su estado y operaciones.
class History:
    # Inicializa la instancia con path.
    def __init__(self, path):
        # Guarda en self.db (conexión SQLite) el resultado de sqlite3.connect, que abre la conexión a la base local.
        self.db = sqlite3.connect(str(path), timeout=0.1)
        # Permite acceder a las columnas de los resultados por su nombre.
        self.db.row_factory = sqlite3.Row
        # Ejecuta el bloque con manejo de excepciones.
        try:
            # Activa la comprobación de claves foráneas en esta conexión.
            self.db.execute("PRAGMA foreign_keys = ON")
            # Lee la versión del esquema almacenada en SQLite.
            version = self.db.execute("PRAGMA user_version").fetchone()[0]
            # Crea el esquema si la base no tiene una versión registrada.
            if version == 0:
                # Ejecuta el bloque de sentencias SQL mediante self.db.executescript con 'BEGIN IMMEDIATE;' + _SCHEMA + 'COMMIT;'.
                self.db.executescript("BEGIN IMMEDIATE;" + _SCHEMA + "COMMIT;")
            # Detecta un formato de historial que esta aplicación no reconoce.
            elif version != 1:
                # Interrumpe la operación con sqlite3.DatabaseError('Versión de historial incompatible.').
                raise sqlite3.DatabaseError("Versión de historial incompatible.")
        # Maneja Exception.
        except Exception:
            # Cierra el recurso mediante self.db.close.
            self.db.close()
            # Vuelve a propagar la excepción en curso.
            raise

    # Define close, que cierra el recurso.
    def close(self):
        # Cierra el recurso mediante self.db.close.
        self.db.close()

    # Define begin con los parámetros session_id, kind, source, backend, mode, profile, slots, interval, motion_area.
    def begin(self, session_id, kind, source, backend, mode, profile, slots, interval, motion_area):
        # Obtiene source_id (identificador de fuente) con '|'.join((kind, backend, source)).
        source_id = "|".join((kind, backend, source))
        # Usa la conexión como contexto para confirmar o revertir la transacción al salir.
        with self.db:
            # Ejecuta la sentencia SQL mediante self.db.execute con 'INSERT OR IGNORE INTO source VALUES (?, ?, ?, ?)', (source_…
            self.db.execute(
                # Aporta el texto "INSERT OR IGNORE INTO source VALUES (?, ?, ?, ?)" a la expresión.
                "INSERT OR IGNORE INTO source VALUES (?, ?, ?, ?)",
                # Aporta (source_id, kind, source, backend) como elemento de la lista.
                (source_id, kind, source, backend),
            )
            # Ejecuta la sentencia SQL mediante self.db.execute con "INSERT INTO session\n (session_id, source_id, started_at_ut…
            self.db.execute(
                """INSERT INTO session
                   (session_id, source_id, started_at_utc, initial_mode, profile,
                    slot_count, sample_interval_s, motion_area_percent, state, version)
                   VALUES (?, ?, ?, ?, ?, ?, ?, ?, 'created', ?)""",
                # Ejecuta (session_id, source_id, utc_now(), mode, profile, slots, interval, motion_area, __versi…
                (session_id, source_id, utc_now(), mode, profile, slots, interval, motion_area, __version__),
            )

    # Define connected con los parámetros session_id, header.
    def connected(self, session_id, header):
        # Usa la conexión como contexto para confirmar o revertir la transacción al salir.
        with self.db:
            # Ejecuta la sentencia SQL mediante self.db.execute con "UPDATE session SET width=?, height=?, pixel_format=?, nomin…
            self.db.execute(
                """UPDATE session SET width=?, height=?, pixel_format=?, nominal_fps=?,
                   state='running' WHERE session_id=?""",
                # Aporta (header.width, header.height, header.pixel_format como elemento de la lista.
                (header.width, header.height, header.pixel_format,
                 # Ejecuta header.fps_numerator / max(1, header.fps_denominator), session_id),.
                 header.fps_numerator / max(1, header.fps_denominator), session_id),
            )

    # Guarda una muestra identificada por su sesión y secuencia de fotograma.
    def sample(self, session_id, elapsed, data):
        # Usa la conexión como contexto para confirmar o revertir la transacción al salir.
        with self.db:
            # Ejecuta la sentencia SQL mediante self.db.execute con 'INSERT OR IGNORE INTO metric_sample VALUES (?, ?, ?, ?, ?,…
            self.db.execute(
                # Aporta el texto "INSERT OR IGNORE INTO metric_sample VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)" a la expresión.
                "INSERT OR IGNORE INTO metric_sample VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)",
                # Aporta (session_id, data["sequence"], elapsed, data["mode"], data["capture_fps"] como elemento de la lista.
                (session_id, data["sequence"], elapsed, data["mode"], data["capture_fps"],
                 # Aporta data["consumer_fps"], data["latency_ms"], data["skipped"], data["overwrites"] como elemento de la lista.
                 data["consumer_fps"], data["latency_ms"], data["skipped"], data["overwrites"],
                 # Aporta data["buffer"], data["detections_min"], data["detections_total"]) como elemento de la lista.
                 data["buffer"], data["detections_min"], data["detections_total"]),
            )

    # Define event con los parámetros session_id, elapsed, level, message.
    def event(self, session_id, elapsed, level, message):
        # Usa la conexión como contexto para confirmar o revertir la transacción al salir.
        with self.db:
            # Ejecuta la sentencia SQL mediante self.db.execute con 'INSERT INTO diagnostic_event (session_id, elapsed_s, level,…
            self.db.execute(
                # Aporta el texto "INSERT INTO diagnostic_event (session_id, elapsed_s, level, message) VALUES (?, ?, ?,… a la expresión.
                "INSERT INTO diagnostic_event (session_id, elapsed_s, level, message) VALUES (?, ?, ?, ?)",
                # Aporta (session_id, elapsed, level, message[:1000]) como elemento de la lista.
                (session_id, elapsed, level, message[:1000]),
            )

    # Actualiza la fecha de cierre, duración y estado final de la sesión.
    def finish(self, session_id, duration, state):
        # Usa la conexión como contexto para confirmar o revertir la transacción al salir.
        with self.db:
            # Ejecuta la sentencia SQL mediante self.db.execute con 'UPDATE session SET ended_at_utc=?, duration_s=?, state=? WH…
            self.db.execute(
                # Aporta el texto "UPDATE session SET ended_at_utc=?, duration_s=?, state=? WHERE session_id=?" a la expresión.
                "UPDATE session SET ended_at_utc=?, duration_s=?, state=? WHERE session_id=?",
                # Ejecuta (utc_now(), duration, state, session_id),.
                (utc_now(), duration, state, session_id),
            )

    # Define sessions sin parámetros de llamada.
    def sessions(self):
        # Devuelve self.db.execute('SELECT session.*, source.display_name, source.backend\n FROM ses…
        return self.db.execute(
            """SELECT session.*, source.display_name, source.backend
               FROM session JOIN source USING(source_id)
               ORDER BY started_at_utc DESC LIMIT 100"""
        # Completa la expresión y encadena fetchall().
        ).fetchall()

    # Consulta los agregados de todas las muestras de la sesión.
    def summary(self, session_id):
        # Devuelve self.db.execute('SELECT COUNT(*) AS samples, AVG(capture_fps) AS capture,\n AVG(p…
        return self.db.execute(
            """SELECT COUNT(*) AS samples, AVG(capture_fps) AS capture,
                      AVG(processing_fps) AS processing, AVG(latency_ms) AS latency,
                      MAX(skipped) AS skipped, MAX(overwrites) AS overwrites,
                      MAX(detections_total) AS detections
               FROM metric_sample WHERE session_id=?""", (session_id,),
        # Completa la expresión y encadena fetchone().
        ).fetchone()

    # Consulta las muestras que usa el gráfico del historial.
    def samples(self, session_id):
        # Devuelve list(reversed(self.db.execute('SELECT * FROM metric_sample WHERE session_id=?\n O…
        return list(reversed(self.db.execute(
            """SELECT * FROM metric_sample WHERE session_id=?
               ORDER BY sequence DESC LIMIT 600""", (session_id,),
        # Completa la expresión y encadena fetchall())).
        ).fetchall()))

    # Define events con el parámetro session_id.
    def events(self, session_id):
        # Devuelve list(reversed(self.db.execute('SELECT elapsed_s, level, message FROM diagnostic_e…
        return list(reversed(self.db.execute(
            """SELECT elapsed_s, level, message FROM diagnostic_event WHERE session_id=?
               ORDER BY event_id DESC LIMIT 50""", (session_id,),
        # Completa la expresión y encadena fetchall())).
        ).fetchall()))
