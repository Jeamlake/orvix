"""Browse local sessions and inspect their recorded metrics."""

# Importa sqlite3.
import sqlite3
# Importa datetime desde datetime.
from datetime import datetime

# Importa QPointF, QRectF, Qt desde PySide6.QtCore.
from PySide6.QtCore import QPointF, QRectF, Qt
# Importa QColor, QPainter, QPen, QPolygonF desde PySide6.QtGui.
from PySide6.QtGui import QColor, QPainter, QPen, QPolygonF
# Importa QComboBox, QHBoxLayout, QPlainTextEdit, QPushButton, QVBoxLayout, QWidget desde PySide6.QtWidgets.
from PySide6.QtWidgets import QComboBox, QHBoxLayout, QPlainTextEdit, QPushButton, QVBoxLayout, QWidget

# Importa label desde orvix.ui.widgets.
from orvix.ui.widgets import label
# Importa MODES desde orvix.vision.pipeline.
from orvix.vision.pipeline import MODES

# Construye el mapa de datos SERIES.
SERIES = {
    # Asocia la clave "FPS" con (("capture_fps", "Captura", "#66e8b3"), ("processing_fps", "Procesamiento", "#bc9…
    "FPS": (("capture_fps", "Captura", "#66e8b3"), ("processing_fps", "Procesamiento", "#bc9cff")),
    # Asocia la clave "Latencia (ms)" con (("latency_ms", "Procesamiento", "#bc9cff"),).
    "Latencia (ms)": (("latency_ms", "Procesamiento", "#bc9cff"),),
    # Asocia la clave "Detecciones / min" con (("detections_min", "Últimos 60 s", "#66e8b3"),).
    "Detecciones / min": (("detections_min", "Últimos 60 s", "#66e8b3"),),
    # Asocia la clave "Ocupación (%)" con (("buffer_percent", "Búfer", "#bc9cff"),).
    "Ocupación (%)": (("buffer_percent", "Búfer", "#bc9cff"),),
}


# Declara el tipo Timeline que hereda de QWidget.
class Timeline(QWidget):
    # Inicializa la instancia y prepara su estado.
    def __init__(self):
        # Ejecuta super().__init__().
        super().__init__()
        # Ejecuta self.setMinimumHeight(155).
        self.setMinimumHeight(155)
        # Ejecuta self.setAccessibleName("Serie temporal de la sesión").
        self.setAccessibleName("Serie temporal de la sesión")
        # Inicializa self.samples con [].
        self.samples = []
        # Establece self.series a partir de SERIES['FPS'].
        self.series = SERIES["FPS"]

    # Define paintEvent con el parámetro _event.
    def paintEvent(self, _event):
        # Obtiene painter con QPainter(self).
        painter = QPainter(self)
        # Ejecuta painter.setRenderHint(QPainter.Antialiasing).
        painter.setRenderHint(QPainter.Antialiasing)
        # Ejecuta painter.fillRect(self.rect(), QColor("#171722")).
        painter.fillRect(self.rect(), QColor("#171722"))
        # Entra en esta rama si len(self.samples) < 2.
        if len(self.samples) < 2:
            # Ejecuta painter.setPen(QColor("#a3a0b8")).
            painter.setPen(QColor("#a3a0b8"))
            # Ejecuta painter.drawText(self.rect(), Qt.AlignCenter, "Se necesitan dos muestras para dibujar l…
            painter.drawText(self.rect(), Qt.AlignCenter, "Se necesitan dos muestras para dibujar la serie.")
            # Sale de la función sin devolver un valor.
            return
        # Obtiene plot con QRectF(53, 30, self.width() - 73, self.height() - 59).
        plot = QRectF(53, 30, self.width() - 73, self.height() - 59)
        # Inicializa (start, end) con (self.samples[0]['elapsed_s'], self.samples[-1]['elapsed_s']).
        start, end = self.samples[0]["elapsed_s"], self.samples[-1]["elapsed_s"]
        # Obtiene duration con max(end - start, 0.001).
        duration = max(end - start, 0.001)
        # Obtiene maximum con max(1, *(row[key] for row in self.samples for key, _, _ in self.seri…
        maximum = max(1, *(row[key] for row in self.samples for key, _, _ in self.series))
        # Recorre (0, 0.5, 1) y toma cada elemento como fraction.
        for fraction in (0, 0.5, 1):
            # Establece y a partir de plot.bottom() - fraction * plot.height().
            y = plot.bottom() - fraction * plot.height()
            # Ejecuta painter.setPen(QColor("#3b3455")).
            painter.setPen(QColor("#3b3455"))
            # Ejecuta painter.drawLine(QPointF(plot.left(), y), QPointF(plot.right(), y)).
            painter.drawLine(QPointF(plot.left(), y), QPointF(plot.right(), y))
            # Ejecuta painter.setPen(QColor("#d0c8e7")).
            painter.setPen(QColor("#d0c8e7"))
            # Ejecuta painter.drawText(QRectF(0, y - 10, 47, 20), Qt.AlignRight | Qt.AlignVCenter, f"{maximum…
            painter.drawText(QRectF(0, y - 10, 47, 20), Qt.AlignRight | Qt.AlignVCenter, f"{maximum*fraction:.1f}")
        # Ejecuta painter.drawText(QRectF(plot.left(), plot.bottom() + 4, plot.width(), 22), Qt.AlignCent…
        painter.drawText(QRectF(plot.left(), plot.bottom() + 4, plot.width(), 22), Qt.AlignCenter,
                         # Incorpora f"{start:.1f} a {end:.1f} s desde el inicio") en esta operación.
                         f"{start:.1f} a {end:.1f} s desde el inicio")
        # Recorre enumerate(self.series) y toma cada elemento como (index, (key, title, color)).
        for index, (key, title, color) in enumerate(self.series):
            # Ejecuta painter.setPen(QPen(QColor(color), 2)).
            painter.setPen(QPen(QColor(color), 2))
            # Ejecuta painter.drawText(QPointF(plot.left() + 120 * index, 19), title).
            painter.drawText(QPointF(plot.left() + 120 * index, 19), title)
            # Obtiene points con QPolygonF([QPointF(plot.left() + (row['elapsed_s'] - start) / durati…
            points = QPolygonF([
                # Ejecuta QPointF(plot.left() + (row["elapsed_s"] - start) / duration * plot.width(),.
                QPointF(plot.left() + (row["elapsed_s"] - start) / duration * plot.width(),
                        # Ejecuta plot.bottom() - row[key] / maximum * plot.height()).
                        plot.bottom() - row[key] / maximum * plot.height())
                # Incorpora for row in self.samples como condición o recorrido de la expresión.
                for row in self.samples
            ])
            # Ejecuta painter.drawPolyline(points).
            painter.drawPolyline(points)


# Declara el tipo HistoryPage que hereda de QWidget.
class HistoryPage(QWidget):
    # Inicializa la instancia con history.
    def __init__(self, history):
        # Ejecuta super().__init__().
        super().__init__()
        # Establece self.history (historial SQLite) a partir de history.
        self.history = history
        # Inicializa self.rows con [].
        self.rows = []
        # Establece self.live_session a partir de None.
        self.live_session = None
        # Obtiene layout con QVBoxLayout(self).
        layout = QVBoxLayout(self)
        # Ejecuta layout.setContentsMargins(0, 0, 0, 0).
        layout.setContentsMargins(0, 0, 0, 0)
        # Agrega el control al diseño de la ventana mediante layout.addWidget con label('Historial de sesiones', 'heading').
        layout.addWidget(label("Historial de sesiones", "heading"))
        # Obtiene row con QHBoxLayout().
        row = QHBoxLayout()
        # Obtiene self.sessions con QComboBox().
        self.sessions = QComboBox()
        # Ejecuta self.sessions.setAccessibleName("Sesión del historial").
        self.sessions.setAccessibleName("Sesión del historial")
        # Conecta el evento con su manejador mediante self.sessions.currentIndexChanged.connect con self._detail.
        self.sessions.currentIndexChanged.connect(self._detail)
        # Agrega el control al diseño de la ventana mediante row.addWidget con self.sessions, 1.
        row.addWidget(self.sessions, 1)
        # Obtiene refresh con QPushButton('Actualizar').
        refresh = QPushButton("Actualizar")
        # Conecta el evento con su manejador mediante refresh.clicked.connect con self.refresh.
        refresh.clicked.connect(self.refresh)
        # Agrega el control al diseño de la ventana mediante row.addWidget con refresh.
        row.addWidget(refresh)
        # Agrega una distribución de controles mediante layout.addLayout con row.
        layout.addLayout(row)
        # Obtiene self.details con label('', 'muted').
        self.details = label("", "muted")
        # Agrega el control al diseño de la ventana mediante layout.addWidget con self.details.
        layout.addWidget(self.details)
        # Obtiene self.metrics con label('', 'section').
        self.metrics = label("", "section")
        # Agrega el control al diseño de la ventana mediante layout.addWidget con self.metrics.
        layout.addWidget(self.metrics)
        # Obtiene self.series con QComboBox().
        self.series = QComboBox()
        # Ejecuta self.series.addItems(SERIES).
        self.series.addItems(SERIES)
        # Ejecuta self.series.setAccessibleName("Métrica del gráfico").
        self.series.setAccessibleName("Métrica del gráfico")
        # Conecta el evento con su manejador mediante self.series.currentTextChanged.connect con self._series_changed.
        self.series.currentTextChanged.connect(self._series_changed)
        # Agrega el control al diseño de la ventana mediante layout.addWidget con self.series.
        layout.addWidget(self.series)
        # Obtiene self.timeline con Timeline().
        self.timeline = Timeline()
        # Agrega el control al diseño de la ventana mediante layout.addWidget con self.timeline, 1.
        layout.addWidget(self.timeline, 1)
        # Agrega el control al diseño de la ventana mediante layout.addWidget con label('Últimos eventos de la sesión', 'section').
        layout.addWidget(label("Últimos eventos de la sesión", "section"))
        # Obtiene self.events (eventos de diagnóstico) con QPlainTextEdit().
        self.events = QPlainTextEdit()
        # Ejecuta self.events.setReadOnly(True).
        self.events.setReadOnly(True)
        # Ejecuta self.events.setAccessibleName("Eventos de la sesión").
        self.events.setAccessibleName("Eventos de la sesión")
        # Agrega el control al diseño de la ventana mediante layout.addWidget con self.events, 1.
        layout.addWidget(self.events, 1)

    # Define refresh sin parámetros de llamada.
    def refresh(self):
        # Obtiene selected con self.sessions.currentData().
        selected = self.sessions.currentData()
        # Ejecuta self.sessions.blockSignals(True).
        self.sessions.blockSignals(True)
        # Vacía el contenedor mediante self.sessions.clear.
        self.sessions.clear()
        # Ejecuta el bloque con manejo de excepciones.
        try:
            # Establece self.rows a partir de self.history.sessions() if self.history else [].
            self.rows = self.history.sessions() if self.history else []
            # Recorre self.rows y toma cada elemento como row.
            for row in self.rows:
                # Obtiene started con datetime.fromisoformat(row['started_at_utc']).astimezone().
                started = datetime.fromisoformat(row["started_at_utc"]).astimezone()
                # Agrega una opción a la lista mediante self.sessions.addItem con f"{started:%d/%m/%Y %H:%M:%S} · {row['display_name']}", row[…
                self.sessions.addItem(f'{started:%d/%m/%Y %H:%M:%S} · {row["display_name"]}', row["session_id"])
            # Obtiene index con self.sessions.findData(selected).
            index = self.sessions.findData(selected)
            # Ejecuta self.sessions.setCurrentIndex(max(0, index)).
            self.sessions.setCurrentIndex(max(0, index))
        # Maneja sqlite3.Error.
        except sqlite3.Error as error:
            # Inicializa self.rows con [].
            self.rows = []
            # Ejecuta self._empty("No se pudo consultar el historial. " + str(error)).
            self._empty("No se pudo consultar el historial. " + str(error))
            # Sale de la función sin devolver un valor.
            return
        # Ejecuta la limpieza al salir del bloque, incluso si hubo un error.
        finally:
            # Ejecuta self.sessions.blockSignals(False).
            self.sessions.blockSignals(False)
        # Ejecuta self._detail(self.sessions.currentIndex()).
        self._detail(self.sessions.currentIndex())

    # Define _empty con el parámetro message.
    def _empty(self, message):
        # Actualiza el texto visible del control mediante self.details.setText con message.
        self.details.setText(message)
        # Vacía el contenedor mediante self.metrics.clear.
        self.metrics.clear()
        # Vacía el contenedor mediante self.events.clear.
        self.events.clear()
        # Inicializa self.timeline.samples con [].
        self.timeline.samples = []
        # Ejecuta self.timeline.update().
        self.timeline.update()

    # Define _detail con el parámetro index.
    def _detail(self, index):
        # Entra en esta rama si index < 0 or index >= len(self.rows).
        if index < 0 or index >= len(self.rows):
            # Ejecuta self._empty("Las capturas aparecerán aquí." if self.history else "Historial local no di…
            self._empty("Las capturas aparecerán aquí." if self.history else "Historial local no disponible.")
            # Sale de la función sin devolver un valor.
            return
        # Establece row a partir de self.rows[index].
        row = self.rows[index]
        # Establece session_id (identificador de sesión) a partir de row['session_id'].
        session_id = row["session_id"]
        # Establece state (estado) a partir de {'created': 'Inicio sin cierre registrado', 'running': 'Sin cierre registrado', '…
        state = {
            # Asocia la clave "created" con "Inicio sin cierre registrado", "running": "Sin cierre registrado".
            "created": "Inicio sin cierre registrado", "running": "Sin cierre registrado",
            # Asocia la clave "stopped" con "Finalizada", "failed": "Fallida", "forced": "Parada forzada".
            "stopped": "Finalizada", "failed": "Fallida", "forced": "Parada forzada",
        # Completa la expresión y encadena [row["state"]].
        }[row["state"]]
        # Entra en esta rama si session_id == self.live_session.
        if session_id == self.live_session:
            # Establece state (estado) a partir de 'En curso'.
            state = "En curso"
        # Establece geometry a partir de f"{row['width']}×{row['height']} · {row['pixel_format']}" if row['width'] else 'S…
        geometry = f'{row["width"]}×{row["height"]} · {row["pixel_format"]}' if row["width"] else "Sin formato negociado"
        # Establece duration a partir de f"{row['duration_s']:.1f} s" if row['duration_s'] is not None else '—'.
        duration = f'{row["duration_s"]:.1f} s' if row["duration_s"] is not None else "—"
        # Actualiza el texto visible del control mediante self.details.setText con f"{state} · {duration} · {row['backend']} · {geometry}\nPerf…
        self.details.setText(f'{state} · {duration} · {row["backend"]} · {geometry}\n'
                             # Aporta el texto f'Perfil {row["profile"]} · inicio en {MODES[row["initial_mode"]]} · ' a la expresión.
                             f'Perfil {row["profile"]} · inicio en {MODES[row["initial_mode"]]} · '
                             # Incorpora f'muestra cada {row["sample_interval_s"]} s · versión {row["version"]}') en esta operación.
                             f'muestra cada {row["sample_interval_s"]} s · versión {row["version"]}')
        # Ejecuta el bloque con manejo de excepciones.
        try:
            # Obtiene los valores agregados de la sesión seleccionada.
            summary = self.history.summary(session_id)
            # Carga las mediciones individuales que dibuja el gráfico.
            self.timeline.samples = self.history.samples(session_id)
            # Recupera los eventos de diagnóstico asociados a la sesión.
            events = self.history.events(session_id)
        # Maneja sqlite3.Error.
        except sqlite3.Error as error:
            # Ejecuta self._empty("No se pudo leer la sesión. " + str(error)).
            self._empty("No se pudo leer la sesión. " + str(error))
            # Sale de la función sin devolver un valor.
            return
        # Entra en esta rama si summary['samples'].
        if summary["samples"]:
            # Actualiza el texto visible del control mediante self.metrics.setText con f"{summary['capture']:.1f} FPS captura · {summary['processin…
            self.metrics.setText(
                # Aporta el texto f'{summary["capture"]:.1f} FPS captura · {summary["processing"]:.1f} FPS proceso · ' a la expresión.
                f'{summary["capture"]:.1f} FPS captura  ·  {summary["processing"]:.1f} FPS proceso  ·  '
                # Aporta el texto f'{summary["latency"]:.2f} ms medio\n' a la expresión.
                f'{summary["latency"]:.2f} ms medio\n'
                # Aporta el texto f'{summary["skipped"]} omitidos · {summary["overwrites"]} sobrescritos · ' a la expresión.
                f'{summary["skipped"]} omitidos  ·  {summary["overwrites"]} sobrescritos  ·  '
                # Incorpora f'{summary["detections"]} detecciones · {summary["samples"]} muestras') en esta operación.
                f'{summary["detections"]} detecciones  ·  {summary["samples"]} muestras')
        # Ejecuta la alternativa si no se tomó la rama anterior.
        else:
            # Actualiza el texto visible del control mediante self.metrics.setText con 'Esta sesión no produjo muestras.'.
            self.metrics.setText("Esta sesión no produjo muestras.")
        # Ejecuta self.events.setPlainText("\n".join(.
        self.events.setPlainText("\n".join(
            # Ejecuta f'{event["elapsed_s"]:7.2f} s {event["level"].upper()} {event["message"]}' for event in…
            f'{event["elapsed_s"]:7.2f} s  {event["level"].upper()}  {event["message"]}' for event in events
        ))
        # Ejecuta self.timeline.update().
        self.timeline.update()

    # Define _series_changed con el parámetro name.
    def _series_changed(self, name):
        # Establece self.timeline.series a partir de SERIES[name].
        self.timeline.series = SERIES[name]
        # Ejecuta self.timeline.update().
        self.timeline.update()
