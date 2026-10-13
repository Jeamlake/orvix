"""Desktop orchestration: C++ owns the camera, a worker consumes IPC frames."""

from __future__ import annotations

import os
import re
import sys
import threading
import time
import uuid
from collections import deque
from dataclasses import dataclass
from pathlib import Path
from typing import Optional

from PySide6.QtCore import QObject, QProcess, QStandardPaths, QThread, QTimer, Signal

from orvix.ipc import SharedMemoryReader
from orvix.ipc.protocol import PRODUCER_FAILED, PRODUCER_STOPPED
from orvix.metrics import SequenceTracker
from orvix.vision import decode_bgr


@dataclass(frozen=True)
class CameraChoice:
    index: int
    name: str
    backend: str


def parse_devices(output: str):
    """Parse the native CLI's indexed enumeration, not camera APIs in Python."""
    matches = re.finditer(r"^\[(\d+)\] (.+)\r?\n\s+Backend: (.+)", output, re.M)
    return [CameraChoice(int(m[1]), m[2].strip(), m[3].strip()) for m in matches]


def find_engine(explicit: Optional[str] = None) -> Path:
    if explicit:
        candidates = [Path(explicit).expanduser().resolve()]
    elif getattr(sys, "frozen", False):
        candidates = [Path(sys._MEIPASS) / "native" / "orvix-capture.exe"]
    else:
        root = Path(__file__).resolve().parents[4]
        name = "orvix-capture.exe" if os.name == "nt" else "orvix-capture"
        candidates = [root / "build/native/Release" / name, root / "build/native" / name]
    for candidate in candidates:
        if candidate.is_file():
            return candidate
    raise FileNotFoundError("No se encontró el motor nativo. Compila ORVIX o reinstala la aplicación.")


class LatestFrame:
    """One-frame mailbox: a slow UI never accumulates queued video frames."""

    def __init__(self):
        self._lock = threading.Lock()
        self._value = None

    def put(self, image, metrics):
        with self._lock:
            self._value = (image, metrics)

    def take(self):
        with self._lock:
            value, self._value = self._value, None
            return value


class FrameWorker(QThread):
    connected = Signal(object)
    failed = Signal(str)

    def __init__(self, name, mailbox, parent=None, attach_timeout=8.0, stall_timeout=5.0):
        super().__init__(parent)
        self.name = name
        self.mailbox = mailbox
        self.attach_timeout = attach_timeout
        self.stall_timeout = stall_timeout

    def run(self):
        reader = None
        try:
            deadline = time.monotonic() + self.attach_timeout
            while not self.isInterruptionRequested():
                try:
                    reader = SharedMemoryReader(self.name, attach_timeout=0)
                    break
                except (TimeoutError, RuntimeError):
                    if time.monotonic() >= deadline:
                        raise TimeoutError("El motor no publicó el puente a tiempo.")
                    self.msleep(50)
            if reader is None:
                return
            initial = reader.header
            self.connected.emit(initial)
            sequences = SequenceTracker()
            samples = deque(maxlen=90)
            last_frame_at = time.monotonic()
            while not self.isInterruptionRequested():
                header = reader.header
                # Do not copy/decode an already-consumed frame every 2 ms.
                if header.published_sequence > sequences.last_sequence:
                    frame = reader.read_latest()
                    if frame is not None and frame.sequence > sequences.last_sequence:
                        image = decode_bgr(frame)
                        sequences.observe(frame.sequence)
                        occupancy = header.buffer_utilization * 100.0
                        reader.acknowledge(frame.sequence)
                        now = time.monotonic()
                        samples.append(now)
                        elapsed = samples[-1] - samples[0]
                        fps = (len(samples) - 1) / elapsed if elapsed > 0 else 0.0
                        self.mailbox.put(image, {
                            "sequence": frame.sequence, "width": frame.width,
                            "height": frame.height, "format": frame.pixel_format,
                            "timestamp": frame.timestamp_100ns, "bytes": len(frame.payload),
                            "capture_fps": header.producer_fps, "consumer_fps": fps,
                            "skipped": sequences.skipped_frames,
                            "overwrites": header.overwritten_frames, "buffer": occupancy,
                        })
                        last_frame_at = now
                if header.producer_state in (PRODUCER_STOPPED, PRODUCER_FAILED):
                    if header.producer_state == PRODUCER_FAILED:
                        raise RuntimeError("El motor indicó un fallo de captura.")
                    break
                if time.monotonic() - last_frame_at > self.stall_timeout:
                    raise TimeoutError("No llegan frames desde hace 5 segundos. Revisa la cámara o detén el motor.")
                self.msleep(2)
        except Exception as error:
            if not self.isInterruptionRequested():
                self.failed.emit(str(error))
        finally:
            if reader is not None:
                reader.close()


class CaptureController(QObject):
    state_changed = Signal(str, str)
    devices_changed = Signal(object)
    diagnostics = Signal(str, str)
    connected = Signal(object)
    quiescent = Signal()

    def __init__(self, engine: Path, parent=None, work_dir=None):
        super().__init__(parent)
        self.engine = engine
        self.work_dir = Path(work_dir or QStandardPaths.writableLocation(QStandardPaths.AppLocalDataLocation))
        self.work_dir.mkdir(parents=True, exist_ok=True)
        self.state = "idle"
        self.mailbox = LatestFrame()
        self.worker = None
        self.name = ""
        self._stopping = False
        self._fault = False
        self._output_tail = ""
        self._closing = False
        self.process = QProcess(self)
        self.process.setWorkingDirectory(str(self.work_dir))
        self.process.setProcessChannelMode(QProcess.MergedChannels)
        self.process.readyReadStandardOutput.connect(self._read_output)
        self.process.started.connect(self._start_reader)
        self.process.finished.connect(self._process_finished)
        self.process.errorOccurred.connect(self._process_error)
        self.enumerator = QProcess(self)
        self.enumerator.setWorkingDirectory(str(self.work_dir))
        self.enumerator.setProcessChannelMode(QProcess.MergedChannels)
        self.enumerator.finished.connect(self._devices_finished)
        self.enumerator.errorOccurred.connect(self._devices_error)
        self.enumeration_timer = QTimer(self)
        self.enumeration_timer.setSingleShot(True)
        self.enumeration_timer.timeout.connect(self._devices_timeout)
        self.stop_timer = QTimer(self)
        self.stop_timer.setSingleShot(True)
        self.stop_timer.timeout.connect(self._force_stop)
        self.kill_timer = QTimer(self)
        self.kill_timer.setSingleShot(True)
        self.kill_timer.timeout.connect(self.process.kill)

    @property
    def active(self):
        return self.process.state() != QProcess.NotRunning or self.worker is not None

    def _state(self, state, text):
        self.state = state
        self.state_changed.emit(state, text)

    def refresh_devices(self):
        if self.active or self.enumerator.state() != QProcess.NotRunning:
            return
        self.enumerator.start(str(self.engine), ["devices"])
        self.enumeration_timer.start(5000)

    def _devices_finished(self, code, _status):
        self.enumeration_timer.stop()
        output = bytes(self.enumerator.readAllStandardOutput()).decode("utf-8", errors="replace")
        self.devices_changed.emit(parse_devices(output) if code == 0 else [])
        if code:
            self.diagnostics.emit("error", "No se pudieron enumerar las cámaras. " + output[-800:])
        elif not parse_devices(output):
            self.diagnostics.emit("info", "No hay cámaras disponibles. Puedes usar la demo sintética.")
        if self._closing and not self.active:
            self.quiescent.emit()

    def _devices_error(self, error):
        if error == QProcess.FailedToStart:
            self.enumeration_timer.stop()
            self.devices_changed.emit([])
            self.diagnostics.emit("error", "No se pudo ejecutar el motor para buscar cámaras.")
            # FailedToStart does not emit finished; allow a pending window close.
            if self._closing and not self.active:
                self.quiescent.emit()

    def _devices_timeout(self):
        self.enumerator.kill()
        self.diagnostics.emit("warning", "La búsqueda de cámaras tardó demasiado. Revisa el dispositivo y vuelve a buscar.")

    def start(self, index=None):
        if self.active or self.enumerator.state() != QProcess.NotRunning:
            return
        self._stopping = self._fault = False
        self.mailbox.take()
        self._output_tail = ""
        # Native IPC names are limited to 30 characters for POSIX portability.
        self.name = "orvix-ui-" + uuid.uuid4().hex[:16]
        args = ["bridge", "--continuous", "--name", self.name]
        args += ["--synthetic"] if index is None else ["--index", str(index)]
        self._state("starting", "Conectando con el motor…")
        self.diagnostics.emit("info", "Nueva captura · " + ("demo sintética" if index is None else "cámara {}".format(index)))
        self.process.start(str(self.engine), args)

    def _start_reader(self):
        if self._stopping:
            return
        self.worker = FrameWorker(self.name, self.mailbox, self)
        self.worker.connected.connect(self._connected)
        self.worker.failed.connect(self._reader_failed)
        self.worker.finished.connect(self._reader_finished)
        self.worker.start()

    def retry_view(self):
        if self.process.state() == QProcess.Running and self.worker is None and not self._stopping:
            self._fault = False
            self._state("starting", "Reconectando la vista…")
            self._start_reader()

    def _connected(self, header):
        if self._stopping:
            return
        self.connected.emit(header)
        self._state("running", "Captura en directo")
        self.diagnostics.emit("success", "Bridge conectado · PID {} · {}×{} · {} · RAW".format(
            header.producer_pid, header.width, header.height, header.pixel_format))

    def _reader_failed(self, message):
        self._fault = True
        self._state("error", "La vista necesita atención")
        self.diagnostics.emit("error", message + " El consumidor no finaliza el motor; puedes reintentar la vista o detenerlo.")

    def _reader_finished(self):
        old_worker = self.worker
        self.worker = None
        if old_worker is not None:
            old_worker.deleteLater()
        self._settle()

    def _read_output(self):
        self._output_tail += bytes(self.process.readAllStandardOutput()).decode("utf-8", errors="replace")
        lines = self._output_tail.split("\n")
        self._output_tail = lines.pop()[-4096:]
        for line in lines:
            line = line.strip()
            if not line:
                continue
            level = "error" if re.search(r"\[ORV-[^]]+\]", line) else "info"
            self.diagnostics.emit(level, line[:1000])

    def _process_error(self, error):
        if error == QProcess.FailedToStart:
            self._fault = True
            self._state("error", "No se pudo iniciar el motor")
            self.diagnostics.emit("error", self.process.errorString())
            self._settle()

    def _process_finished(self, code, status):
        self.stop_timer.stop()
        self.kill_timer.stop()
        self._read_output()
        if self.worker is not None:
            self.worker.requestInterruption()
        if not self._stopping and (code or status == QProcess.CrashExit):
            self._fault = True
            self.diagnostics.emit("error", "El motor terminó inesperadamente (código {}). Revisa el diagnóstico antes de reintentar.".format(code))
        self._settle()

    def stop(self):
        if not self.active:
            return
        self._stopping = True
        self._state("stopping", "Deteniendo y liberando la cámara…")
        if self.worker is not None:
            self.worker.requestInterruption()
        try:
            with SharedMemoryReader(self.name, 0) as reader:
                reader.request_stop()
        except (RuntimeError, TimeoutError, OSError):
            pass  # Startup can be cancelled before shared memory exists.
        if self.process.state() != QProcess.NotRunning:
            self.stop_timer.start(3000)
        self._settle()

    def _force_stop(self):
        if self.process.state() != QProcess.NotRunning:
            self.diagnostics.emit("warning", "El motor no respondió a la parada. Se cierra únicamente el proceso de esta captura.")
            self.process.terminate()
            self.kill_timer.start(500)

    def _settle(self):
        if not self.active:
            self._state("error" if self._fault and not self._stopping else "idle",
                        "Revisa el diagnóstico y vuelve a iniciar" if self._fault and not self._stopping else "Lista para una nueva captura")
            self._stopping = False
            self.quiescent.emit()

    def shutdown(self):
        self._closing = True
        self.enumeration_timer.stop()
        if self.enumerator.state() != QProcess.NotRunning:
            self.enumerator.kill()
        self.stop()
        # The executable can fail synchronously before shutdown is requested.
        if not self.active and self.enumerator.state() == QProcess.NotRunning:
            self.quiescent.emit()
