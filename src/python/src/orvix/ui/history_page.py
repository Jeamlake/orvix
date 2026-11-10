"""Browse local sessions and inspect their recorded metrics."""

import sqlite3
from datetime import datetime

from PySide6.QtCore import QPointF, QRectF, Qt
from PySide6.QtGui import QColor, QPainter, QPen, QPolygonF
from PySide6.QtWidgets import QComboBox, QHBoxLayout, QPlainTextEdit, QPushButton, QVBoxLayout, QWidget

from orvix.ui.widgets import label
from orvix.vision.pipeline import MODES

SERIES = {
    "FPS": (("capture_fps", "Captura", "#66e8b3"), ("processing_fps", "Procesamiento", "#bc9cff")),
    "Latencia (ms)": (("latency_ms", "Procesamiento", "#bc9cff"),),
    "Detecciones / min": (("detections_min", "Últimos 60 s", "#66e8b3"),),
    "Ocupación (%)": (("buffer_percent", "Búfer", "#bc9cff"),),
}


class Timeline(QWidget):
    def __init__(self):
        super().__init__()
        self.setMinimumHeight(155)
        self.setAccessibleName("Serie temporal de la sesión")
        self.samples = []
        self.series = SERIES["FPS"]

    def paintEvent(self, _event):
        painter = QPainter(self)
        painter.setRenderHint(QPainter.Antialiasing)
        painter.fillRect(self.rect(), QColor("#171722"))
        if len(self.samples) < 2:
            painter.setPen(QColor("#a3a0b8"))
            painter.drawText(self.rect(), Qt.AlignCenter, "Se necesitan dos muestras para dibujar la serie.")
            return
        plot = QRectF(53, 30, self.width() - 73, self.height() - 59)
        start, end = self.samples[0]["elapsed_s"], self.samples[-1]["elapsed_s"]
        duration = max(end - start, 0.001)
        maximum = max(1, *(row[key] for row in self.samples for key, _, _ in self.series))
        for fraction in (0, 0.5, 1):
            y = plot.bottom() - fraction * plot.height()
            painter.setPen(QColor("#3b3455"))
            painter.drawLine(QPointF(plot.left(), y), QPointF(plot.right(), y))
            painter.setPen(QColor("#d0c8e7"))
            painter.drawText(QRectF(0, y - 10, 47, 20), Qt.AlignRight | Qt.AlignVCenter, f"{maximum*fraction:.1f}")
        painter.drawText(QRectF(plot.left(), plot.bottom() + 4, plot.width(), 22), Qt.AlignCenter,
                         f"{start:.1f} a {end:.1f} s desde el inicio")
        for index, (key, title, color) in enumerate(self.series):
            painter.setPen(QPen(QColor(color), 2))
            painter.drawText(QPointF(plot.left() + 120 * index, 19), title)
            points = QPolygonF([
                QPointF(plot.left() + (row["elapsed_s"] - start) / duration * plot.width(),
                        plot.bottom() - row[key] / maximum * plot.height())
                for row in self.samples
            ])
            painter.drawPolyline(points)


class HistoryPage(QWidget):
    def __init__(self, history):
        super().__init__()
        self.history = history
        self.rows = []
        self.live_session = None
        layout = QVBoxLayout(self)
        layout.setContentsMargins(0, 0, 0, 0)
        layout.addWidget(label("Historial de sesiones", "heading"))
        row = QHBoxLayout()
        self.sessions = QComboBox()
        self.sessions.setAccessibleName("Sesión del historial")
        self.sessions.currentIndexChanged.connect(self._detail)
        row.addWidget(self.sessions, 1)
        refresh = QPushButton("Actualizar")
        refresh.clicked.connect(self.refresh)
        row.addWidget(refresh)
        layout.addLayout(row)
        self.details = label("", "muted")
        layout.addWidget(self.details)
        self.metrics = label("", "section")
        layout.addWidget(self.metrics)
        self.series = QComboBox()
        self.series.addItems(SERIES)
        self.series.setAccessibleName("Métrica del gráfico")
        self.series.currentTextChanged.connect(self._series_changed)
        layout.addWidget(self.series)
        self.timeline = Timeline()
        layout.addWidget(self.timeline, 1)
        layout.addWidget(label("Últimos eventos de la sesión", "section"))
        self.events = QPlainTextEdit()
        self.events.setReadOnly(True)
        self.events.setAccessibleName("Eventos de la sesión")
        layout.addWidget(self.events, 1)

    def refresh(self):
        selected = self.sessions.currentData()
        self.sessions.blockSignals(True)
        self.sessions.clear()
        try:
            self.rows = self.history.sessions() if self.history else []
            for row in self.rows:
                started = datetime.fromisoformat(row["started_at_utc"]).astimezone()
                self.sessions.addItem(f'{started:%d/%m/%Y %H:%M:%S} · {row["display_name"]}', row["session_id"])
            index = self.sessions.findData(selected)
            self.sessions.setCurrentIndex(max(0, index))
        except sqlite3.Error as error:
            self.rows = []
            self._empty("No se pudo consultar el historial. " + str(error))
            return
        finally:
            self.sessions.blockSignals(False)
        self._detail(self.sessions.currentIndex())

    def _empty(self, message):
        self.details.setText(message)
        self.metrics.clear()
        self.events.clear()
        self.timeline.samples = []
        self.timeline.update()

    def _detail(self, index):
        if index < 0 or index >= len(self.rows):
            self._empty("Las capturas aparecerán aquí." if self.history else "Historial local no disponible.")
            return
        row = self.rows[index]
        session_id = row["session_id"]
        state = {
            "created": "Inicio sin cierre registrado", "running": "Sin cierre registrado",
            "stopped": "Finalizada", "failed": "Fallida", "forced": "Parada forzada",
        }[row["state"]]
        if session_id == self.live_session:
            state = "En curso"
        geometry = f'{row["width"]}×{row["height"]} · {row["pixel_format"]}' if row["width"] else "Sin formato negociado"
        duration = f'{row["duration_s"]:.1f} s' if row["duration_s"] is not None else "—"
        self.details.setText(f'{state} · {duration} · {row["backend"]} · {geometry}\n'
                             f'Perfil {row["profile"]} · inicio en {MODES[row["initial_mode"]]} · '
                             f'muestra cada {row["sample_interval_s"]} s · versión {row["version"]}')
        try:
            summary = self.history.summary(session_id)
            self.timeline.samples = self.history.samples(session_id)
            events = self.history.events(session_id)
        except sqlite3.Error as error:
            self._empty("No se pudo leer la sesión. " + str(error))
            return
        if summary["samples"]:
            self.metrics.setText(
                f'{summary["capture"]:.1f} FPS captura  ·  {summary["processing"]:.1f} FPS proceso  ·  '
                f'{summary["latency"]:.2f} ms medio\n'
                f'{summary["skipped"]} omitidos  ·  {summary["overwrites"]} sobrescritos  ·  '
                f'{summary["detections"]} detecciones  ·  {summary["samples"]} muestras')
        else:
            self.metrics.setText("Esta sesión no produjo muestras.")
        self.events.setPlainText("\n".join(
            f'{event["elapsed_s"]:7.2f} s  {event["level"].upper()}  {event["message"]}' for event in events
        ))
        self.timeline.update()

    def _series_changed(self, name):
        self.timeline.series = SERIES[name]
        self.timeline.update()
