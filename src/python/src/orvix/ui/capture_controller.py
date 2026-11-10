"""Desktop orchestration: C++ owns the camera, a worker consumes IPC frames."""

# Importa annotations desde __future__.
from __future__ import annotations

# Importa os.
import os
# Importa re.
import re
# Importa sys.
import sys
# Importa threading.
import threading
# Importa time.
import time
# Importa uuid.
import uuid
# Importa deque desde collections.
from collections import deque
# Importa dataclass desde dataclasses.
from dataclasses import dataclass
# Importa Path desde pathlib.
from pathlib import Path
# Importa Optional desde typing.
from typing import Optional

# Importa QObject, QProcess, QStandardPaths, QThread, QTimer, Signal desde PySide6.QtCore.
from PySide6.QtCore import QObject, QProcess, QStandardPaths, QThread, QTimer, Signal

# Importa SharedMemoryReader desde orvix.ipc.
from orvix.ipc import SharedMemoryReader
# Importa PRODUCER_FAILED, PRODUCER_STOPPED desde orvix.ipc.protocol.
from orvix.ipc.protocol import PRODUCER_FAILED, PRODUCER_STOPPED
# Importa SequenceTracker desde orvix.metrics.
from orvix.metrics import SequenceTracker
# Importa decode_bgr desde orvix.vision.
from orvix.vision import decode_bgr
# Importa VisionPipeline desde orvix.vision.pipeline.
from orvix.vision.pipeline import VisionPipeline

# Construye el mapa de datos PROFILES.
PROFILES = {
    # Asocia la clave "480p30" con (640, 480, 30).
    "480p30": (640, 480, 30),
    # Asocia la clave "720p30" con (1280, 720, 30).
    "720p30": (1280, 720, 30),
    # Asocia la clave "1080p30" con (1920, 1080, 30).
    "1080p30": (1920, 1080, 30),
}


# Aplica el decorador dataclass(frozen=True) a la definición siguiente.
@dataclass(frozen=True)
# Declara el tipo CameraChoice para agrupar su estado y operaciones.
class CameraChoice:
    # Declara el tipo de index.
    index: int
    # Declara el tipo de name.
    name: str
    # Declara el tipo de backend (implementación nativa).
    backend: str


# Define parse_devices con el parámetro output.
def parse_devices(output: str):
    """Parse the native CLI's indexed enumeration, not camera APIs in Python."""
    # Obtiene matches con re.finditer('^\\[(\\d+)\\] (.+)\\r?\\n\\s+Backend: (.+)', output, re…
    matches = re.finditer(r"^\[(\d+)\] (.+)\r?\n\s+Backend: (.+)", output, re.M)
    # Devuelve [CameraChoice(int(m[1]), m[2].strip(), m[3].strip()) for m in matches].
    return [CameraChoice(int(m[1]), m[2].strip(), m[3].strip()) for m in matches]


# Busca el ejecutable C++ en la ruta indicada, el paquete o la compilación local.
def find_engine(explicit: Optional[str] = None) -> Path:
    # Da prioridad a una ruta del motor que proporcionó el usuario.
    if explicit:
        # Inicializa candidates con [Path(explicit).expanduser().resolve()].
        candidates = [Path(explicit).expanduser().resolve()]
    # Entra en esta rama si getattr(sys, 'frozen', False).
    elif getattr(sys, "frozen", False):
        # Inicializa candidates con [Path(sys._MEIPASS) / 'native' / 'orvix-capture.exe'].
        candidates = [Path(sys._MEIPASS) / "native" / "orvix-capture.exe"]
    # Ejecuta la alternativa si no se tomó la rama anterior.
    else:
        # Establece root a partir de Path(__file__).resolve().parents[5].
        root = Path(__file__).resolve().parents[5]
        # Establece name a partir de 'orvix-capture.exe' if os.name == 'nt' else 'orvix-capture'.
        name = "orvix-capture.exe" if os.name == "nt" else "orvix-capture"
        # Inicializa candidates con [root / 'build/native/Release' / name, root / 'build/native' / name].
        candidates = [root / "build/native/Release" / name, root / "build/native" / name]
    # Recorre candidates y toma cada elemento como candidate.
    for candidate in candidates:
        # Entra en esta rama si candidate.is_file().
        if candidate.is_file():
            # Devuelve candidate.
            return candidate
    # Interrumpe la operación con FileNotFoundError('No se encontró el motor nativo. Compila ORVIX o reinstala la aplicac…
    raise FileNotFoundError("No se encontró el motor nativo. Compila ORVIX o reinstala la aplicación.")


# Declara el tipo LatestFrame para agrupar su estado y operaciones.
class LatestFrame:
    """One-frame mailbox: a slow UI never accumulates queued video frames."""

    # Inicializa la instancia y prepara su estado.
    def __init__(self):
        # Obtiene self._lock con threading.Lock().
        self._lock = threading.Lock()
        # Establece self._value a partir de None.
        self._value = None

    # Define put con los parámetros image, metrics.
    def put(self, image, metrics):
        # Acota el uso de self._lock a este bloque.
        with self._lock:
            # Inicializa self._value con (image, metrics).
            self._value = (image, metrics)

    # Define take sin parámetros de llamada.
    def take(self):
        # Acota el uso de self._lock a este bloque.
        with self._lock:
            # Inicializa (value, self._value) con (self._value, None).
            value, self._value = self._value, None
            # Devuelve value.
            return value


# Declara el tipo FrameWorker que hereda de QThread.
class FrameWorker(QThread):
    # Obtiene connected con Signal(object).
    connected = Signal(object)
    # Obtiene failed con Signal(str).
    failed = Signal(str)

    # Inicializa la instancia con name, mailbox, sequences, pipeline, mode, parent, attach_timeout, stall_timeout.
    def __init__(self, name, mailbox, sequences, pipeline, mode, parent=None, attach_timeout=8.0, stall_timeout=5.0):
        # Ejecuta super().__init__(parent).
        super().__init__(parent)
        # Establece self.name a partir de name.
        self.name = name
        # Establece self.mailbox a partir de mailbox.
        self.mailbox = mailbox
        # Establece self.attach_timeout a partir de attach_timeout.
        self.attach_timeout = attach_timeout
        # Establece self.stall_timeout a partir de stall_timeout.
        self.stall_timeout = stall_timeout
        # Establece self.sequences a partir de sequences.
        self.sequences = sequences
        # Establece self.pipeline a partir de pipeline.
        self.pipeline = pipeline
        # A single string is exchanged with the UI. The worker owns pipeline state.
        # Guarda el modo de procesamiento seleccionado.
        self.mode = mode

    # Define run sin parámetros de llamada.
    def run(self):
        # Establece reader (lector de fotogramas) a partir de None.
        reader = None
        # Ejecuta el bloque con manejo de excepciones.
        try:
            # Establece deadline a partir de time.monotonic() + self.attach_timeout.
            deadline = time.monotonic() + self.attach_timeout
            # Repite mientras not self.isInterruptionRequested().
            while not self.isInterruptionRequested():
                # Ejecuta el bloque con manejo de excepciones.
                try:
                    # Obtiene reader (lector de fotogramas) con SharedMemoryReader(self.name, attach_timeout=0).
                    reader = SharedMemoryReader(self.name, attach_timeout=0)
                    # Termina el bucle actual.
                    break
                # Maneja (TimeoutError, RuntimeError).
                except (TimeoutError, RuntimeError):
                    # Entra en esta rama si time.monotonic() >= deadline.
                    if time.monotonic() >= deadline:
                        # Interrumpe la operación con TimeoutError('El motor no publicó el puente a tiempo.').
                        raise TimeoutError("El motor no publicó el puente a tiempo.")
                    # Ejecuta self.msleep(50).
                    self.msleep(50)
            # Entra en esta rama si reader is None.
            if reader is None:
                # Sale de la función sin devolver un valor.
                return
            # Establece initial a partir de reader.header.
            initial = reader.header
            # Publica la señal para sus receptores mediante self.connected.emit con initial.
            self.connected.emit(initial)
            # Establece sequences a partir de self.sequences.
            sequences = self.sequences
            # Ejecuta self.pipeline.reset_reference().
            self.pipeline.reset_reference()
            # Obtiene samples con deque(maxlen=90).
            samples = deque(maxlen=90)
            # Guarda en last_frame_at el resultado de time.monotonic, que lee un reloj monotónico.
            last_frame_at = time.monotonic()
            # Repite mientras not self.isInterruptionRequested().
            while not self.isInterruptionRequested():
                # Establece header a partir de reader.header.
                header = reader.header
                # Do not copy/decode an already-consumed frame every 2 ms.
                # Entra en esta rama si header.published_sequence > sequences.last_sequence.
                if header.published_sequence > sequences.last_sequence:
                    # Obtiene frame (fotograma) con reader.read_latest().
                    frame = reader.read_latest()
                    # Entra en esta rama si frame is not None and frame.sequence > sequences.last_sequence.
                    if frame is not None and frame.sequence > sequences.last_sequence:
                        # Establece mode (modo de procesamiento) a partir de self.mode.
                        mode = self.mode
                        # Obtiene started con time.perf_counter().
                        started = time.perf_counter()
                        # Obtiene image (imagen) con self.pipeline.process(decode_bgr(frame), mode).
                        image = self.pipeline.process(decode_bgr(frame), mode)
                        # Establece latency a partir de (time.perf_counter() - started) * 1000.
                        latency = (time.perf_counter() - started) * 1000
                        # Ejecuta sequences.observe(frame.sequence).
                        sequences.observe(frame.sequence)
                        # Establece occupancy a partir de header.buffer_utilization * 100.0.
                        occupancy = header.buffer_utilization * 100.0
                        # Ejecuta reader.acknowledge(frame.sequence).
                        reader.acknowledge(frame.sequence)
                        # Guarda en now el resultado de time.monotonic, que lee un reloj monotónico.
                        now = time.monotonic()
                        # Agrega un elemento al final mediante samples.append con now.
                        samples.append(now)
                        # Establece elapsed (tiempo transcurrido) a partir de samples[-1] - samples[0].
                        elapsed = samples[-1] - samples[0]
                        # Establece fps a partir de (len(samples) - 1) / elapsed if elapsed > 0 else 0.0.
                        fps = (len(samples) - 1) / elapsed if elapsed > 0 else 0.0
                        # Ejecuta self.mailbox.put(image, {.
                        self.mailbox.put(image, {
                            # Asocia la clave "sequence" con frame.sequence, "width": frame.width.
                            "sequence": frame.sequence, "width": frame.width,
                            # Asocia la clave "height" con frame.height, "format": frame.pixel_format.
                            "height": frame.height, "format": frame.pixel_format,
                            # Asocia la clave "timestamp" con frame.timestamp_100ns, "bytes": len(frame.payload).
                            "timestamp": frame.timestamp_100ns, "bytes": len(frame.payload),
                            # Asocia la clave "capture_fps" con header.producer_fps, "consumer_fps": fps.
                            "capture_fps": header.producer_fps, "consumer_fps": fps,
                            # Asocia la clave "skipped" con sequences.skipped_frames.
                            "skipped": sequences.skipped_frames,
                            # Asocia la clave "overwrites" con header.overwritten_frames, "buffer": occupancy.
                            "overwrites": header.overwritten_frames, "buffer": occupancy,
                            # Asocia la clave "mode" con mode, "latency_ms": latency, "slots": header.slot_count.
                            "mode": mode, "latency_ms": latency, "slots": header.slot_count,
                            # Asocia la clave "detections_min" con len(self.pipeline.detection_times).
                            "detections_min": len(self.pipeline.detection_times),
                            # Asocia la clave "detections_total" con self.pipeline.total_detections.
                            "detections_total": self.pipeline.total_detections,
                        })
                        # Establece last_frame_at a partir de now.
                        last_frame_at = now
                # Entra en esta rama si header.producer_state in (PRODUCER_STOPPED, PRODUCER_FAILED).
                if header.producer_state in (PRODUCER_STOPPED, PRODUCER_FAILED):
                    # Entra en esta rama si header.producer_state == PRODUCER_FAILED.
                    if header.producer_state == PRODUCER_FAILED:
                        # Interrumpe la operación con RuntimeError('El motor indicó un fallo de captura.').
                        raise RuntimeError("El motor indicó un fallo de captura.")
                    # Termina el bucle actual.
                    break
                # Entra en esta rama si time.monotonic() - last_frame_at > self.stall_timeout.
                if time.monotonic() - last_frame_at > self.stall_timeout:
                    # Interrumpe la operación con TimeoutError('No llegan frames desde hace 5 segundos. Revisa la cámara o detén el motor…
                    raise TimeoutError("No llegan frames desde hace 5 segundos. Revisa la cámara o detén el motor.")
                # Ejecuta self.msleep(2).
                self.msleep(2)
        # Maneja Exception.
        except Exception as error:
            # Entra en esta rama si not self.isInterruptionRequested().
            if not self.isInterruptionRequested():
                # Publica la señal para sus receptores mediante self.failed.emit con str(error).
                self.failed.emit(str(error))
        # Ejecuta la limpieza al salir del bloque, incluso si hubo un error.
        finally:
            # Entra en esta rama si reader is not None.
            if reader is not None:
                # Cierra el recurso mediante reader.close.
                reader.close()


# Declara el tipo CaptureController que hereda de QObject.
class CaptureController(QObject):
    # Obtiene state_changed con Signal(str, str).
    state_changed = Signal(str, str)
    # Obtiene devices_changed con Signal(object).
    devices_changed = Signal(object)
    # Obtiene diagnostics con Signal(str, str).
    diagnostics = Signal(str, str)
    # Obtiene connected con Signal(object).
    connected = Signal(object)
    # Obtiene quiescent con Signal().
    quiescent = Signal()

    # Inicializa la instancia con engine, parent, work_dir.
    def __init__(self, engine: Path, parent=None, work_dir=None):
        # Ejecuta super().__init__(parent).
        super().__init__(parent)
        # Establece self.engine a partir de engine.
        self.engine = engine
        # Obtiene self.work_dir con Path(work_dir or QStandardPaths.writableLocation(QStandardPaths.AppL…
        self.work_dir = Path(work_dir or QStandardPaths.writableLocation(QStandardPaths.AppLocalDataLocation))
        # Ejecuta self.work_dir.mkdir(parents=True, exist_ok=True).
        self.work_dir.mkdir(parents=True, exist_ok=True)
        # Establece self.state (estado) a partir de 'idle'.
        self.state = "idle"
        # Obtiene self.mailbox con LatestFrame().
        self.mailbox = LatestFrame()
        # Establece self.worker a partir de None.
        self.worker = None
        # Establece self.name a partir de ''.
        self.name = ""
        # Establece self.mode (modo de procesamiento) a partir de 'raw'.
        self.mode = "raw"
        # Establece self.profile a partir de '720p30'.
        self.profile = "720p30"
        # Establece self.slots (posiciones del anillo) a partir de 3.
        self.slots = 3
        # Establece self.motion_area a partir de 0.25.
        self.motion_area = 0.25
        # Establece self.forced_stop a partir de False.
        self.forced_stop = False
        # Establece self._stopping a partir de False.
        self._stopping = False
        # Establece self._fault a partir de False.
        self._fault = False
        # Establece self._output_tail a partir de ''.
        self._output_tail = ""
        # Establece self._closing a partir de False.
        self._closing = False
        # Obtiene self.process con QProcess(self).
        self.process = QProcess(self)
        # Ejecuta self.process.setWorkingDirectory(str(self.work_dir)).
        self.process.setWorkingDirectory(str(self.work_dir))
        # Ejecuta self.process.setProcessChannelMode(QProcess.MergedChannels).
        self.process.setProcessChannelMode(QProcess.MergedChannels)
        # Conecta el evento con su manejador mediante self.process.readyReadStandardOutput.connect con self._read_output.
        self.process.readyReadStandardOutput.connect(self._read_output)
        # Conecta el evento con su manejador mediante self.process.started.connect con self._start_reader.
        self.process.started.connect(self._start_reader)
        # Conecta el evento con su manejador mediante self.process.finished.connect con self._process_finished.
        self.process.finished.connect(self._process_finished)
        # Conecta el evento con su manejador mediante self.process.errorOccurred.connect con self._process_error.
        self.process.errorOccurred.connect(self._process_error)
        # Obtiene self.enumerator con QProcess(self).
        self.enumerator = QProcess(self)
        # Ejecuta self.enumerator.setWorkingDirectory(str(self.work_dir)).
        self.enumerator.setWorkingDirectory(str(self.work_dir))
        # Ejecuta self.enumerator.setProcessChannelMode(QProcess.MergedChannels).
        self.enumerator.setProcessChannelMode(QProcess.MergedChannels)
        # Conecta el evento con su manejador mediante self.enumerator.finished.connect con self._devices_finished.
        self.enumerator.finished.connect(self._devices_finished)
        # Conecta el evento con su manejador mediante self.enumerator.errorOccurred.connect con self._devices_error.
        self.enumerator.errorOccurred.connect(self._devices_error)
        # Obtiene self.enumeration_timer con QTimer(self).
        self.enumeration_timer = QTimer(self)
        # Ejecuta self.enumeration_timer.setSingleShot(True).
        self.enumeration_timer.setSingleShot(True)
        # Conecta el evento con su manejador mediante self.enumeration_timer.timeout.connect con self._devices_timeout.
        self.enumeration_timer.timeout.connect(self._devices_timeout)
        # Obtiene self.stop_timer con QTimer(self).
        self.stop_timer = QTimer(self)
        # Ejecuta self.stop_timer.setSingleShot(True).
        self.stop_timer.setSingleShot(True)
        # Conecta el evento con su manejador mediante self.stop_timer.timeout.connect con self._force_stop.
        self.stop_timer.timeout.connect(self._force_stop)
        # Obtiene self.kill_timer con QTimer(self).
        self.kill_timer = QTimer(self)
        # Ejecuta self.kill_timer.setSingleShot(True).
        self.kill_timer.setSingleShot(True)
        # Conecta el evento con su manejador mediante self.kill_timer.timeout.connect con self.process.kill.
        self.kill_timer.timeout.connect(self.process.kill)

    # Aplica el decorador property a la definición siguiente.
    @property
    # Define active sin parámetros de llamada.
    def active(self):
        # Devuelve self.process.state() != QProcess.NotRunning or self.worker is not None.
        return self.process.state() != QProcess.NotRunning or self.worker is not None

    # Define _state con los parámetros state, text.
    def _state(self, state, text):
        # Establece self.state (estado) a partir de state.
        self.state = state
        # Publica la señal para sus receptores mediante self.state_changed.emit con state, text.
        self.state_changed.emit(state, text)

    # Define refresh_devices sin parámetros de llamada.
    def refresh_devices(self):
        # Entra en esta rama si self._closing or self.active or self.enumerator.state() != QProcess.NotRunning.
        if self._closing or self.active or self.enumerator.state() != QProcess.NotRunning:
            # Sale de la función sin devolver un valor.
            return
        # Ejecuta self.enumerator.start(str(self.engine), ["devices"]).
        self.enumerator.start(str(self.engine), ["devices"])
        # Ejecuta self.enumeration_timer.start(5000).
        self.enumeration_timer.start(5000)

    # Define _devices_finished con los parámetros code, _status.
    def _devices_finished(self, code, _status):
        # Ejecuta self.enumeration_timer.stop().
        self.enumeration_timer.stop()
        # Obtiene output con bytes(self.enumerator.readAllStandardOutput()).decode('utf-8', error…
        output = bytes(self.enumerator.readAllStandardOutput()).decode("utf-8", errors="replace")
        # Publica la señal para sus receptores mediante self.devices_changed.emit con parse_devices(output) if code == 0 else [].
        self.devices_changed.emit(parse_devices(output) if code == 0 else [])
        # Entra en esta rama si code.
        if code:
            # Publica la señal para sus receptores mediante self.diagnostics.emit con 'error', 'No se pudieron enumerar las cámaras. ' + output[-8…
            self.diagnostics.emit("error", "No se pudieron enumerar las cámaras. " + output[-800:])
        # Entra en esta rama si not parse_devices(output).
        elif not parse_devices(output):
            # Publica la señal para sus receptores mediante self.diagnostics.emit con 'info', 'No hay cámaras disponibles. Puedes usar la demo sin…
            self.diagnostics.emit("info", "No hay cámaras disponibles. Puedes usar la demo sintética.")
        # Entra en esta rama si self._closing and (not self.active).
        if self._closing and not self.active:
            # Publica la señal para sus receptores mediante self.quiescent.emit.
            self.quiescent.emit()

    # Define _devices_error con el parámetro error.
    def _devices_error(self, error):
        # Entra en esta rama si error == QProcess.FailedToStart.
        if error == QProcess.FailedToStart:
            # Ejecuta self.enumeration_timer.stop().
            self.enumeration_timer.stop()
            # Publica la señal para sus receptores mediante self.devices_changed.emit con [].
            self.devices_changed.emit([])
            # Publica la señal para sus receptores mediante self.diagnostics.emit con 'error', 'No se pudo ejecutar el motor para buscar cámaras.'.
            self.diagnostics.emit("error", "No se pudo ejecutar el motor para buscar cámaras.")
            # FailedToStart does not emit finished; allow a pending window close.
            # Entra en esta rama si self._closing and (not self.active).
            if self._closing and not self.active:
                # Publica la señal para sus receptores mediante self.quiescent.emit.
                self.quiescent.emit()

    # Define _devices_timeout sin parámetros de llamada.
    def _devices_timeout(self):
        # Ejecuta self.enumerator.kill().
        self.enumerator.kill()
        # Publica la señal para sus receptores mediante self.diagnostics.emit con 'warning', 'La búsqueda de cámaras tardó demasiado. Revisa e…
        self.diagnostics.emit("warning", "La búsqueda de cámaras tardó demasiado. Revisa el dispositivo y vuelve a buscar.")

    # Define start con el parámetro index.
    def start(self, index=None):
        # Entra en esta rama si self._closing or self.active or self.enumerator.state() != QProcess.NotRunning.
        if self._closing or self.active or self.enumerator.state() != QProcess.NotRunning:
            # Sale de la función sin devolver un valor.
            return
        # Establece self._stopping, self._fault a partir de False.
        self._stopping = self._fault = False
        # Establece self.forced_stop a partir de False.
        self.forced_stop = False
        # Obtiene self.sequences con SequenceTracker().
        self.sequences = SequenceTracker()
        # Obtiene self.pipeline con VisionPipeline(self.motion_area).
        self.pipeline = VisionPipeline(self.motion_area)
        # Ejecuta self.mailbox.take().
        self.mailbox.take()
        # Establece self._output_tail a partir de ''.
        self._output_tail = ""
        # Native IPC names are limited to 30 characters for POSIX portability.
        # Establece self.name a partir de 'orvix-ui-' + uuid.uuid4().hex[:16].
        self.name = "orvix-ui-" + uuid.uuid4().hex[:16]
        # Inicializa args con ['bridge', '--continuous', '--name', self.name].
        args = ["bridge", "--continuous", "--name", self.name]
        # Establece (width, height, fps) a partir de PROFILES[self.profile].
        width, height, fps = PROFILES[self.profile]
        # Suma ['--width', str(width), '--height', str(height), '--fps', str(fps), '--slots', st… al valor de args.
        args += ["--width", str(width), "--height", str(height), "--fps", str(fps), "--slots", str(self.slots)]
        # Suma ['--synthetic'] if index is None else ['--index', str(index)] al valor de args.
        args += ["--synthetic"] if index is None else ["--index", str(index)]
        # Ejecuta self._state("starting", "Conectando con el motor…").
        self._state("starting", "Conectando con el motor…")
        # Publica la señal para sus receptores mediante self.diagnostics.emit con 'info', 'Nueva captura · ' + ('demo sintética' if index is N…
        self.diagnostics.emit("info", "Nueva captura · " + ("demo sintética" if index is None else "cámara {}".format(index)))
        # Lanza el ejecutable C++ como un proceso separado con los argumentos de la sesión.
        self.process.start(str(self.engine), args)

    # Define set_mode, que actualiza el filtro de procesamiento.
    def set_mode(self, mode):
        # Guarda el modo de procesamiento seleccionado.
        self.mode = mode
        # Entra en esta rama si self.worker is not None.
        if self.worker is not None:
            # Cambia el filtro del lector sin reiniciar el proceso de captura.
            self.worker.mode = mode

    # Define _start_reader sin parámetros de llamada.
    def _start_reader(self):
        # Entra en esta rama si self._stopping.
        if self._stopping:
            # Sale de la función sin devolver un valor.
            return
        # Obtiene self.worker con FrameWorker(self.name, self.mailbox, self.sequences, self.pipeline,…
        self.worker = FrameWorker(self.name, self.mailbox, self.sequences, self.pipeline, self.mode, self)
        # Conecta el evento con su manejador mediante self.worker.connected.connect con self._connected.
        self.worker.connected.connect(self._connected)
        # Conecta el evento con su manejador mediante self.worker.failed.connect con self._reader_failed.
        self.worker.failed.connect(self._reader_failed)
        # Conecta el evento con su manejador mediante self.worker.finished.connect con self._reader_finished.
        self.worker.finished.connect(self._reader_finished)
        # Ejecuta self.worker.start().
        self.worker.start()

    # Define retry_view sin parámetros de llamada.
    def retry_view(self):
        # Entra en esta rama si self.process.state() == QProcess.Running and self.worker is None and (not self._stoppin…
        if self.process.state() == QProcess.Running and self.worker is None and not self._stopping:
            # Establece self._fault a partir de False.
            self._fault = False
            # Ejecuta self._state("starting", "Reconectando la vista…").
            self._state("starting", "Reconectando la vista…")
            # Ejecuta self._start_reader().
            self._start_reader()

    # Define _connected con el parámetro header.
    def _connected(self, header):
        # Entra en esta rama si self._stopping.
        if self._stopping:
            # Sale de la función sin devolver un valor.
            return
        # Publica la señal para sus receptores mediante self.connected.emit con header.
        self.connected.emit(header)
        # Ejecuta self._state("running", "Captura en directo").
        self._state("running", "Captura en directo")
        # Publica la señal para sus receptores mediante self.diagnostics.emit con 'success', 'Bridge conectado · PID {} · {}×{} · {}'.format(h…
        self.diagnostics.emit("success", "Bridge conectado · PID {} · {}×{} · {}".format(
            # Incorpora header.producer_pid, header.width, header.height, header.pixel_format)) en esta operación.
            header.producer_pid, header.width, header.height, header.pixel_format))

    # Define _reader_failed con el parámetro message.
    def _reader_failed(self, message):
        # Establece self._fault a partir de True.
        self._fault = True
        # Ejecuta self._state("error", "La vista necesita atención").
        self._state("error", "La vista necesita atención")
        # Publica la señal para sus receptores mediante self.diagnostics.emit con 'error', message + ' El consumidor no finaliza el motor; pue…
        self.diagnostics.emit("error", message + " El consumidor no finaliza el motor; puedes reintentar la vista o detenerlo.")

    # Define _reader_finished sin parámetros de llamada.
    def _reader_finished(self):
        # Establece old_worker a partir de self.worker.
        old_worker = self.worker
        # Establece self.worker a partir de None.
        self.worker = None
        # Entra en esta rama si old_worker is not None.
        if old_worker is not None:
            # Ejecuta old_worker.deleteLater().
            old_worker.deleteLater()
        # Ejecuta self._settle().
        self._settle()

    # Define _read_output sin parámetros de llamada.
    def _read_output(self):
        # Suma bytes(self.process.readAllStandardOutput()).decode('utf-8', errors='replace') al valor de self._output_tail.
        self._output_tail += bytes(self.process.readAllStandardOutput()).decode("utf-8", errors="replace")
        # Obtiene lines con self._output_tail.split('\n').
        lines = self._output_tail.split("\n")
        # Establece self._output_tail a partir de lines.pop()[-4096:].
        self._output_tail = lines.pop()[-4096:]
        # Recorre lines y toma cada elemento como line.
        for line in lines:
            # Obtiene line con line.strip().
            line = line.strip()
            # Entra en esta rama si not line.
            if not line:
                # Omite lo restante y pasa a la siguiente iteración.
                continue
            # Establece level a partir de 'error' if re.search('\\[ORV-[^]]+\\]', line) else 'info'.
            level = "error" if re.search(r"\[ORV-[^]]+\]", line) else "info"
            # Publica la señal para sus receptores mediante self.diagnostics.emit con level, line[:1000].
            self.diagnostics.emit(level, line[:1000])

    # Define _process_error con el parámetro error.
    def _process_error(self, error):
        # Entra en esta rama si error == QProcess.FailedToStart.
        if error == QProcess.FailedToStart:
            # Establece self._fault a partir de True.
            self._fault = True
            # Ejecuta self._state("error", "No se pudo iniciar el motor").
            self._state("error", "No se pudo iniciar el motor")
            # Publica la señal para sus receptores mediante self.diagnostics.emit con 'error', self.process.errorString().
            self.diagnostics.emit("error", self.process.errorString())
            # Ejecuta self._settle().
            self._settle()

    # Define _process_finished con los parámetros code, status.
    def _process_finished(self, code, status):
        # Ejecuta self.stop_timer.stop().
        self.stop_timer.stop()
        # Ejecuta self.kill_timer.stop().
        self.kill_timer.stop()
        # Ejecuta self._read_output().
        self._read_output()
        # Entra en esta rama si self.worker is not None.
        if self.worker is not None:
            # Ejecuta self.worker.requestInterruption().
            self.worker.requestInterruption()
        # Entra en esta rama si not self._stopping and (code or status == QProcess.CrashExit).
        if not self._stopping and (code or status == QProcess.CrashExit):
            # Establece self._fault a partir de True.
            self._fault = True
            # Publica la señal para sus receptores mediante self.diagnostics.emit con 'error', 'El motor terminó inesperadamente (código {}). Revi…
            self.diagnostics.emit("error", "El motor terminó inesperadamente (código {}). Revisa el diagnóstico antes de reintentar.".format(code))
        # Ejecuta self._settle().
        self._settle()

    # Define stop sin parámetros de llamada.
    def stop(self):
        # Entra en esta rama si not self.active.
        if not self.active:
            # Sale de la función sin devolver un valor.
            return
        # Establece self._stopping a partir de True.
        self._stopping = True
        # Ejecuta self._state("stopping", "Deteniendo y liberando la cámara…").
        self._state("stopping", "Deteniendo y liberando la cámara…")
        # Entra en esta rama si self.worker is not None.
        if self.worker is not None:
            # Ejecuta self.worker.requestInterruption().
            self.worker.requestInterruption()
        # Ejecuta el bloque con manejo de excepciones.
        try:
            # Acota el uso de SharedMemoryReader(self.name, 0) a este bloque.
            with SharedMemoryReader(self.name, 0) as reader:
                # Ejecuta reader.request_stop().
                reader.request_stop()
        # Maneja (RuntimeError, TimeoutError, OSError).
        except (RuntimeError, TimeoutError, OSError):
            # Incorpora pass # Startup can be cancelled before shared memory exists. en esta operación.
            pass  # Startup can be cancelled before shared memory exists.
        # Entra en esta rama si self.process.state() != QProcess.NotRunning.
        if self.process.state() != QProcess.NotRunning:
            # Ejecuta self.stop_timer.start(3000).
            self.stop_timer.start(3000)
        # Ejecuta self._settle().
        self._settle()

    # Define _force_stop sin parámetros de llamada.
    def _force_stop(self):
        # Entra en esta rama si self.process.state() != QProcess.NotRunning.
        if self.process.state() != QProcess.NotRunning:
            # Establece self.forced_stop a partir de True.
            self.forced_stop = True
            # Publica la señal para sus receptores mediante self.diagnostics.emit con 'warning', 'El motor no respondió a la parada. Se cierra úni…
            self.diagnostics.emit("warning", "El motor no respondió a la parada. Se cierra únicamente el proceso de esta captura.")
            # Ejecuta self.process.terminate().
            self.process.terminate()
            # Ejecuta self.kill_timer.start(500).
            self.kill_timer.start(500)

    # Define _settle sin parámetros de llamada.
    def _settle(self):
        # Entra en esta rama si not self.active.
        if not self.active:
            # Ejecuta self._state("error" if self._fault and not self._stopping else "idle",.
            self._state("error" if self._fault and not self._stopping else "idle",
                        # Incorpora "Revisa el diagnóstico y vuelve a iniciar" if self._fault and not self._stopping else "… en esta operación.
                        "Revisa el diagnóstico y vuelve a iniciar" if self._fault and not self._stopping else "Lista para una nueva captura")
            # Establece self._stopping a partir de False.
            self._stopping = False
            # Publica la señal para sus receptores mediante self.quiescent.emit.
            self.quiescent.emit()

    # Define shutdown sin parámetros de llamada.
    def shutdown(self):
        # Establece self._closing a partir de True.
        self._closing = True
        # Ejecuta self.enumeration_timer.stop().
        self.enumeration_timer.stop()
        # Entra en esta rama si self.enumerator.state() != QProcess.NotRunning.
        if self.enumerator.state() != QProcess.NotRunning:
            # Ejecuta self.enumerator.kill().
            self.enumerator.kill()
        # Ejecuta self.stop().
        self.stop()
        # The executable can fail synchronously before shutdown is requested.
        # Entra en esta rama si not self.active and self.enumerator.state() == QProcess.NotRunning.
        if not self.active and self.enumerator.state() == QProcess.NotRunning:
            # Publica la señal para sus receptores mediante self.quiescent.emit.
            self.quiescent.emit()
