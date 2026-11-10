"""ORVIX desktop application."""

# Importa annotations desde __future__.
from __future__ import annotations

# Importa argparse.
import argparse
# Importa sqlite3.
import sqlite3
# Importa sys.
import sys
# Importa time.
import time
# Importa uuid.
import uuid
# Importa datetime desde datetime.
from datetime import datetime
# Importa Path desde pathlib.
from pathlib import Path

# Importa QProcess, QSettings, QTimer, Qt desde PySide6.QtCore.
from PySide6.QtCore import QProcess, QSettings, QTimer, Qt
# Importa QCloseEvent, QIcon, QKeySequence, QShortcut desde PySide6.QtGui.
from PySide6.QtGui import QCloseEvent, QIcon, QKeySequence, QShortcut
# Importa QApplication, QButtonGroup, QComboBox, QFileDialog, QFrame, QFormLayout, QHBoxLayout, QMainWindow, QMessageBox, QPlainTextEdit, QProgressBar, QPushButton, QScrollArea, QSizePolicy, QSpinBox, QDoubleSpinBox, QSplitter, QStackedWidget, QVBoxLayout, QWidget desde PySide6.QtWidgets.
from PySide6.QtWidgets import (
    # Aporta QApplication, QButtonGroup, QComboBox, QFileDialog, QFrame, QFormLayout, QHBoxLay… como elemento de la lista.
    QApplication, QButtonGroup, QComboBox, QFileDialog, QFrame, QFormLayout, QHBoxLayout,
    # Aporta QMainWindow, QMessageBox, QPlainTextEdit, QProgressBar, QPushButton como elemento de la lista.
    QMainWindow, QMessageBox, QPlainTextEdit, QProgressBar, QPushButton,
    # Aporta QScrollArea, QSizePolicy, QSpinBox, QDoubleSpinBox, QSplitter, QStackedWidget, QV… como elemento de la lista.
    QScrollArea, QSizePolicy, QSpinBox, QDoubleSpinBox, QSplitter, QStackedWidget, QVBoxLayout, QWidget,
)

# Importa __version__ desde orvix.
from orvix import __version__
# Importa History desde orvix.metrics.history.
from orvix.metrics.history import History
# Importa CaptureController, PROFILES, find_engine desde orvix.ui.capture_controller.
from orvix.ui.capture_controller import CaptureController, PROFILES, find_engine
# Importa HistoryPage desde orvix.ui.history_page.
from orvix.ui.history_page import HistoryPage
# Importa MODES desde orvix.vision.pipeline.
from orvix.vision.pipeline import MODES
# Importa MetricCard, Preview, STYLE, card, label desde orvix.ui.widgets.
from orvix.ui.widgets import MetricCard, Preview, STYLE, card, label


# Define asset_path con el parámetro name.
def asset_path(name):
    # Devuelve Path(__file__).parent / 'assets' / name.
    return Path(__file__).parent / "assets" / name


# Define button con los parámetros text, callback, name, tooltip.
def button(text, callback=None, name=None, tooltip=None):
    # Obtiene widget con QPushButton(text).
    widget = QPushButton(text)
    # Entra en esta rama si name.
    if name:
        # Ejecuta widget.setObjectName(name).
        widget.setObjectName(name)
    # Entra en esta rama si tooltip.
    if tooltip:
        # Ejecuta widget.setToolTip(tooltip).
        widget.setToolTip(tooltip)
    # Entra en esta rama si callback.
    if callback:
        # Conecta el evento con su manejador mediante widget.clicked.connect con callback.
        widget.clicked.connect(callback)
    # Devuelve widget.
    return widget


# Declara el tipo MainWindow que hereda de QMainWindow.
class MainWindow(QMainWindow):
    # Inicializa la instancia con engine, work_dir, settings.
    def __init__(self, engine, work_dir=None, settings=None):
        # Ejecuta super().__init__().
        super().__init__()
        # Ejecuta self.setWindowTitle("ORVIX · " + __version__).
        self.setWindowTitle("ORVIX · " + __version__)
        # Ejecuta self.setWindowIcon(QIcon(str(asset_path("orvix.svg")))).
        self.setWindowIcon(QIcon(str(asset_path("orvix.svg"))))
        # Ejecuta self.setMinimumSize(1024, 680).
        self.setMinimumSize(1024, 680)
        # Ajusta el tamaño del contenedor mediante self.resize con 1320, 860.
        self.resize(1320, 860)
        # Establece self.settings (preferencias de la aplicación) a partir de settings if settings is not None else QSettings().
        self.settings = settings if settings is not None else QSettings()
        # Obtiene self.controller con CaptureController(engine, self, work_dir).
        self.controller = CaptureController(engine, self, work_dir)
        # Establece self.history (historial SQLite) a partir de None.
        self.history = None
        # Establece self._history_problem a partir de None.
        self._history_problem = None
        # Establece self._session_id (identificador de sesión) a partir de None.
        self._session_id = None
        # Establece self._sample_after a partir de 0.0.
        self._sample_after = 0.0
        # Ejecuta el bloque con manejo de excepciones.
        try:
            # Obtiene self.history (historial SQLite) con History(self.controller.work_dir / 'telemetry.sqlite3').
            self.history = History(self.controller.work_dir / "telemetry.sqlite3")
        # Maneja sqlite3.Error.
        except sqlite3.Error as error:
            # Obtiene self._history_problem con str(error).
            self._history_problem = str(error)
        # Establece self._close_when_idle a partir de False.
        self._close_when_idle = False
        # Establece self._started_at a partir de None.
        self._started_at = None
        # Establece self._last_metrics a partir de None.
        self._last_metrics = None
        # Establece self._state_text a partir de 'Lista para capturar'.
        self._state_text = "Lista para capturar"
        # Ejecuta self._build().
        self._build()
        # Ejecuta self.setStyleSheet(STYLE).
        self.setStyleSheet(STYLE)
        # Obtiene geometry con self.settings.value('window/geometry').
        geometry = self.settings.value("window/geometry")
        # Entra en esta rama si geometry.
        if geometry:
            # Ejecuta self.restoreGeometry(geometry).
            self.restoreGeometry(geometry)
        # Conecta el evento con su manejador mediante self.controller.state_changed.connect con self._state_changed.
        self.controller.state_changed.connect(self._state_changed)
        # Conecta el evento con su manejador mediante self.controller.devices_changed.connect con self._devices_changed.
        self.controller.devices_changed.connect(self._devices_changed)
        # Conecta el evento con su manejador mediante self.controller.diagnostics.connect con self._log.
        self.controller.diagnostics.connect(self._log)
        # Conecta el evento con su manejador mediante self.controller.connected.connect con self._connected.
        self.controller.connected.connect(self._connected)
        # Conecta el evento con su manejador mediante self.controller.quiescent.connect con self._quiescent.
        self.controller.quiescent.connect(self._quiescent)
        # Obtiene self.timer con QTimer(self).
        self.timer = QTimer(self)
        # Conecta el evento con su manejador mediante self.timer.timeout.connect con self._render_latest.
        self.timer.timeout.connect(self._render_latest)
        # Ejecuta self.timer.start(33).
        self.timer.start(33)
        # Inicializa self._shortcuts con [].
        self._shortcuts = []
        # Recorre (('Ctrl+Return', self._start), ('Escape', self._stop), ('F11', self._fullscreen), ('Ctr… y toma cada elemento como (key, callback).
        for key, callback in (("Ctrl+Return", self._start), ("Escape", self._stop),
                              # Inicia ("F11", self._fullscreen), ("Ctrl+0", self.preview.fit)): con shortcut = QShortcut(QKeySequence(key), self).
                              ("F11", self._fullscreen), ("Ctrl+0", self.preview.fit)):
            # Obtiene shortcut con QShortcut(QKeySequence(key), self).
            shortcut = QShortcut(QKeySequence(key), self)
            # Conecta el evento con su manejador mediante shortcut.activated.connect con callback.
            shortcut.activated.connect(callback)
            # Agrega un elemento al final mediante self._shortcuts.append con shortcut.
            self._shortcuts.append(shortcut)
        # Ejecuta self._state_changed("idle", "Lista para capturar").
        self._state_changed("idle", "Lista para capturar")
        # Ejecuta self._log("info", "ORVIX guarda métricas y diagnósticos locales. No guarda imágenes ni…
        self._log("info", "ORVIX guarda métricas y diagnósticos locales. No guarda imágenes ni video.")
        # Entra en esta rama si self._history_problem.
        if self._history_problem:
            # Ejecuta self._log("warning", "Historial local no disponible. " + self._history_problem).
            self._log("warning", "Historial local no disponible. " + self._history_problem)
        # Ejecuta QTimer.singleShot(100, self.controller.refresh_devices).
        QTimer.singleShot(100, self.controller.refresh_devices)

    # Define _build sin parámetros de llamada.
    def _build(self):
        # Obtiene shell con QWidget().
        shell = QWidget()
        # Ejecuta shell.setObjectName("shell").
        shell.setObjectName("shell")
        # Ejecuta self.setCentralWidget(shell).
        self.setCentralWidget(shell)
        # Obtiene outer con QHBoxLayout(shell).
        outer = QHBoxLayout(shell)
        # Ejecuta outer.setContentsMargins(0, 0, 0, 0).
        outer.setContentsMargins(0, 0, 0, 0)
        # Ejecuta outer.setSpacing(0).
        outer.setSpacing(0)
        # Obtiene side con QFrame().
        side = QFrame()
        # Ejecuta side.setObjectName("sidebar").
        side.setObjectName("sidebar")
        # Ejecuta side.setFixedWidth(190).
        side.setFixedWidth(190)
        # Obtiene nav con QVBoxLayout(side).
        nav = QVBoxLayout(side)
        # Ejecuta nav.setContentsMargins(18, 28, 18, 24).
        nav.setContentsMargins(18, 28, 18, 24)
        # Ejecuta nav.setSpacing(10).
        nav.setSpacing(10)
        # Obtiene brand con label('◉ ORVIX', 'heading').
        brand = label("◉  ORVIX", "heading")
        # Ejecuta brand.setStyleSheet("color: #c1a5ff; font-size: 25px; letter-spacing: 2px;").
        brand.setStyleSheet("color: #c1a5ff; font-size: 25px; letter-spacing: 2px;")
        # Agrega el control al diseño de la ventana mediante nav.addWidget con brand.
        nav.addWidget(brand)
        # Agrega el control al diseño de la ventana mediante nav.addWidget con label('CAPTURE / VISION / INSIGHT', 'muted').
        nav.addWidget(label("CAPTURE / VISION / INSIGHT", "muted"))
        # Ejecuta nav.addSpacing(34).
        nav.addSpacing(34)
        # Obtiene self.pages con QStackedWidget().
        self.pages = QStackedWidget()
        # Obtiene self.nav_group con QButtonGroup(self).
        self.nav_group = QButtonGroup(self)
        # Recorre enumerate(('◉ Captura', '▤ Historial', '≡ Diagnóstico', '⚙ Ajustes', 'ⓘ Acerca de')) y toma cada elemento como (idx, title).
        for idx, title in enumerate(("◉   Captura", "▤   Historial", "≡   Diagnóstico", "⚙   Ajustes", "ⓘ   Acerca de")):
            # Obtiene item con button(title, name='nav').
            item = button(title, name="nav")
            # Ejecuta item.setCheckable(True).
            item.setCheckable(True)
            # Ejecuta self.nav_group.addButton(item, idx).
            self.nav_group.addButton(item, idx)
            # Conecta el evento con su manejador mediante item.clicked.connect con lambda _checked=False, i=idx: self._navigate(i).
            item.clicked.connect(lambda _checked=False, i=idx: self._navigate(i))
            # Agrega el control al diseño de la ventana mediante nav.addWidget con item.
            nav.addWidget(item)
            # Entra en esta rama si idx == 0.
            if idx == 0:
                # Ejecuta item.setChecked(True).
                item.setChecked(True)
        # Ejecuta nav.addStretch().
        nav.addStretch()
        # Obtiene note con label('Frames en memoria.\nMétricas en este equipo.', 'muted').
        note = label("Frames en memoria.\nMétricas en este equipo.", "muted")
        # Agrega el control al diseño de la ventana mediante nav.addWidget con note.
        nav.addWidget(note)
        # Ejecuta nav.addSpacing(12).
        nav.addSpacing(12)
        # Agrega el control al diseño de la ventana mediante nav.addWidget con label('Versión ' + __version__, 'muted').
        nav.addWidget(label("Versión " + __version__, "muted"))
        # Agrega el control al diseño de la ventana mediante outer.addWidget con side.
        outer.addWidget(side)
        # Obtiene body con QVBoxLayout().
        body = QVBoxLayout()
        # Ejecuta body.setContentsMargins(26, 24, 26, 20).
        body.setContentsMargins(26, 24, 26, 20)
        # Ejecuta body.setSpacing(18).
        body.setSpacing(18)
        # Obtiene header con QHBoxLayout().
        header = QHBoxLayout()
        # Obtiene titles con QVBoxLayout().
        titles = QVBoxLayout()
        # Ejecuta titles.setSpacing(4).
        titles.setSpacing(4)
        # Agrega el control al diseño de la ventana mediante titles.addWidget con label('Tu cámara. Bajo control.', 'heading').
        titles.addWidget(label("Tu cámara. Bajo control.", "heading"))
        # Agrega el control al diseño de la ventana mediante titles.addWidget con label('Captura nativa, memoria compartida y una vista clara…
        titles.addWidget(label("Captura nativa, memoria compartida y una vista clara del proceso.", "muted"))
        # Agrega una distribución de controles mediante header.addLayout con titles, 1.
        header.addLayout(titles, 1)
        # Obtiene self.status con label('Lista para capturar').
        self.status = label("Lista para capturar")
        # Ejecuta self.status.setObjectName("status").
        self.status.setObjectName("status")
        # Agrega el control al diseño de la ventana mediante header.addWidget con self.status, 0.
        header.addWidget(self.status, 0, Qt.AlignTop)
        # Agrega una distribución de controles mediante body.addLayout con header.
        body.addLayout(header)
        # Agrega el control al diseño de la ventana mediante body.addWidget con self.pages, 1.
        body.addWidget(self.pages, 1)
        # Obtiene footer con label('C++ captura → Bridge transporta → NumPy interpreta → OpenCV p…
        footer = label("C++ captura   →   Bridge transporta   →   NumPy interpreta   →   OpenCV procesa   →   Qt muestra", "muted")
        # Ejecuta footer.setStyleSheet("font-size: 11px;").
        footer.setStyleSheet("font-size: 11px;")
        # Agrega el control al diseño de la ventana mediante body.addWidget con footer.
        body.addWidget(footer)
        # Agrega una distribución de controles mediante outer.addLayout con body, 1.
        outer.addLayout(body, 1)
        # Ejecuta self._capture_page().
        self._capture_page()
        # Obtiene self.history_page con HistoryPage(self.history).
        self.history_page = HistoryPage(self.history)
        # Agrega el control al diseño de la ventana mediante self.pages.addWidget con self.history_page.
        self.pages.addWidget(self.history_page)
        # Ejecuta self._diagnostics_page().
        self._diagnostics_page()
        # Ejecuta self._settings_page().
        self._settings_page()
        # Ejecuta self._about_page().
        self._about_page()

    # Define _navigate con el parámetro index.
    def _navigate(self, index):
        # Entra en esta rama si self.pages.widget(index) is self.history_page.
        if self.pages.widget(index) is self.history_page:
            # Ejecuta self.history_page.refresh().
            self.history_page.refresh()
        # Ejecuta self.pages.setCurrentIndex(index).
        self.pages.setCurrentIndex(index)

    # Define _capture_page sin parámetros de llamada.
    def _capture_page(self):
        # Obtiene page con QWidget().
        page = QWidget()
        # Obtiene layout con QVBoxLayout(page).
        layout = QVBoxLayout(page)
        # Ejecuta layout.setContentsMargins(0, 0, 0, 0).
        layout.setContentsMargins(0, 0, 0, 0)
        # Ejecuta layout.setSpacing(14).
        layout.setSpacing(14)
        # Obtiene source (fuente de video) con card().
        source = card()
        # Obtiene row con QHBoxLayout(source).
        row = QHBoxLayout(source)
        # Ejecuta row.setContentsMargins(16, 12, 16, 12).
        row.setContentsMargins(16, 12, 16, 12)
        # Agrega el control al diseño de la ventana mediante row.addWidget con label('Fuente', 'section').
        row.addWidget(label("Fuente", "section"))
        # Obtiene self.source (fuente de video) con QComboBox().
        self.source = QComboBox()
        # Ejecuta self.source.setAccessibleName("Fuente de captura").
        self.source.setAccessibleName("Fuente de captura")
        # Agrega una opción a la lista mediante self.source.addItem con 'Demo sintética · sin cámara', None.
        self.source.addItem("Demo sintética · sin cámara", None)
        # Ejecuta self.source.setMinimumWidth(180).
        self.source.setMinimumWidth(180)
        # Ejecuta self.source.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Fixed).
        self.source.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Fixed)
        # Agrega el control al diseño de la ventana mediante row.addWidget con self.source, 1.
        row.addWidget(self.source, 1)
        # Obtiene self.refresh con button('Buscar', self.controller.refresh_devices, tooltip='Busca dis…
        self.refresh = button("Buscar", self.controller.refresh_devices, tooltip="Busca dispositivos con el motor C++. No abre la cámara.")
        # Obtiene self.start con button('▶ Iniciar', self._start, 'primary', 'Inicia la fuente selecc…
        self.start = button("▶  Iniciar", self._start, "primary", "Inicia la fuente seleccionada · Ctrl+Enter")
        # Obtiene self.stop con button('■ Detener', self._stop, 'danger', 'Libera la cámara y el pue…
        self.stop = button("■  Detener", self._stop, "danger", "Libera la cámara y el puente · Esc")
        # Agrega el control al diseño de la ventana mediante row.addWidget con self.refresh.
        row.addWidget(self.refresh)
        # Agrega el control al diseño de la ventana mediante row.addWidget con self.start.
        row.addWidget(self.start)
        # Agrega el control al diseño de la ventana mediante row.addWidget con self.stop.
        row.addWidget(self.stop)
        # Agrega el control al diseño de la ventana mediante layout.addWidget con source.
        layout.addWidget(source)
        # Obtiene splitter con QSplitter(Qt.Horizontal).
        splitter = QSplitter(Qt.Horizontal)
        # Obtiene viewer con card().
        viewer = card()
        # Obtiene video_layout con QVBoxLayout(viewer).
        video_layout = QVBoxLayout(viewer)
        # Ejecuta video_layout.setContentsMargins(12, 12, 12, 12).
        video_layout.setContentsMargins(12, 12, 12, 12)
        # Ejecuta video_layout.setSpacing(10).
        video_layout.setSpacing(10)
        # Obtiene toolbar con QHBoxLayout().
        toolbar = QHBoxLayout()
        # Agrega el control al diseño de la ventana mediante toolbar.addWidget con label('VISTA EN DIRECTO', 'section'), 1.
        toolbar.addWidget(label("VISTA EN DIRECTO", "section"), 1)
        # Obtiene self.mode (modo de procesamiento) con QComboBox().
        self.mode = QComboBox()
        # Ejecuta self.mode.setAccessibleName("Modo de procesamiento").
        self.mode.setAccessibleName("Modo de procesamiento")
        # Recorre MODES.items() y toma cada elemento como (key, name).
        for key, name in MODES.items():
            # Agrega una opción a la lista mediante self.mode.addItem con name, key.
            self.mode.addItem(name, key)
        # Ejecuta self.mode.setCurrentIndex(max(0, self.mode.findData(self.settings.value("vision/mode",…
        self.mode.setCurrentIndex(max(0, self.mode.findData(self.settings.value("vision/mode", "raw"))))
        # Actualiza el filtro de procesamiento mediante self.controller.set_mode con self.mode.currentData().
        self.controller.set_mode(self.mode.currentData())
        # Conecta el evento con su manejador mediante self.mode.currentIndexChanged.connect con self._mode_changed.
        self.mode.currentIndexChanged.connect(self._mode_changed)
        # Agrega el control al diseño de la ventana mediante toolbar.addWidget con self.mode.
        toolbar.addWidget(self.mode)
        # Agrega el control al diseño de la ventana mediante toolbar.addWidget con button('−', lambda: self.preview.set_zoom(self.preview.zoom…
        toolbar.addWidget(button("−", lambda: self.preview.set_zoom(self.preview.zoom / 1.25), tooltip="Alejar"))
        # Agrega el control al diseño de la ventana mediante toolbar.addWidget con button('+', lambda: self.preview.set_zoom(self.preview.zoom…
        toolbar.addWidget(button("+", lambda: self.preview.set_zoom(self.preview.zoom * 1.25), tooltip="Ampliar"))
        # Agrega el control al diseño de la ventana mediante toolbar.addWidget con button('Ajustar', lambda: self.preview.fit()).
        toolbar.addWidget(button("Ajustar", lambda: self.preview.fit()))
        # Agrega una distribución de controles mediante video_layout.addLayout con toolbar.
        video_layout.addLayout(toolbar)
        # Obtiene self.preview con Preview().
        self.preview = Preview()
        # Agrega el control al diseño de la ventana mediante video_layout.addWidget con self.preview, 1.
        video_layout.addWidget(self.preview, 1)
        # Obtiene self.metadata con label('Sin frames · ajusta la vista con la rueda o Ctrl+0', 'muted').
        self.metadata = label("Sin frames · ajusta la vista con la rueda o Ctrl+0", "muted")
        # Ejecuta self.metadata.setStyleSheet("font-size: 11px;").
        self.metadata.setStyleSheet("font-size: 11px;")
        # Agrega el control al diseño de la ventana mediante video_layout.addWidget con self.metadata.
        video_layout.addWidget(self.metadata)
        # Agrega el control al diseño de la ventana mediante splitter.addWidget con viewer.
        splitter.addWidget(viewer)
        # Obtiene info con card().
        info = card()
        # Ejecuta info.setMinimumWidth(235).
        info.setMinimumWidth(235)
        # Ejecuta info.setMaximumWidth(310).
        info.setMaximumWidth(310)
        # Obtiene panel con QVBoxLayout(info).
        panel = QVBoxLayout(info)
        # Ejecuta panel.setContentsMargins(18, 18, 18, 18).
        panel.setContentsMargins(18, 18, 18, 18)
        # Ejecuta panel.setSpacing(12).
        panel.setSpacing(12)
        # Agrega el control al diseño de la ventana mediante panel.addWidget con label('Estado del puente', 'section').
        panel.addWidget(label("Estado del puente", "section"))
        # Obtiene self.pipeline con label('○ Motor nativo\n\n○ Memoria compartida\n\n○ Procesamiento', '…
        self.pipeline = label("○  Motor nativo\n\n○  Memoria compartida\n\n○  Procesamiento", "muted")
        # Agrega el control al diseño de la ventana mediante panel.addWidget con self.pipeline.
        panel.addWidget(self.pipeline)
        # Obtiene self.session_info con label('Ninguna captura activa\n\nObjetivo inicial: 1280×720 / 30 FPS…
        self.session_info = label("Ninguna captura activa\n\nObjetivo inicial: 1280×720 / 30 FPS.\nEl formato final depende de la cámara.", "muted")
        # Agrega el control al diseño de la ventana mediante panel.addWidget con self.session_info.
        panel.addWidget(self.session_info)
        # Agrega el control al diseño de la ventana mediante panel.addWidget con label('Ocupación antes de consumir', 'muted').
        panel.addWidget(label("Ocupación antes de consumir", "muted"))
        # Obtiene self.buffer_bar con QProgressBar().
        self.buffer_bar = QProgressBar()
        # Establece los límites del control mediante self.buffer_bar.setRange con 0, 100.
        self.buffer_bar.setRange(0, 100)
        # Establece el valor del control o preferencia mediante self.buffer_bar.setValue con 0.
        self.buffer_bar.setValue(0)
        # Ejecuta self.buffer_bar.setTextVisible(False).
        self.buffer_bar.setTextVisible(False)
        # Agrega el control al diseño de la ventana mediante panel.addWidget con self.buffer_bar.
        panel.addWidget(self.buffer_bar)
        # Obtiene self.buffer_info con label('Sin datos del búfer', 'muted').
        self.buffer_info = label("Sin datos del búfer", "muted")
        # Agrega el control al diseño de la ventana mediante panel.addWidget con self.buffer_info.
        panel.addWidget(self.buffer_info)
        # Obtiene self.vision_info con label('Latencia de procesamiento · —\nDetecciones en 60 s · —', 'mut…
        self.vision_info = label("Latencia de procesamiento · —\nDetecciones en 60 s · —", "muted")
        # Ejecuta self.vision_info.setToolTip("Tiempo de decodificación y filtro. En movimiento se regist…
        self.vision_info.setToolTip("Tiempo de decodificación y filtro. En movimiento se registra como máximo una detección por segundo.")
        # Agrega el control al diseño de la ventana mediante panel.addWidget con self.vision_info.
        panel.addWidget(self.vision_info)
        # Ejecuta panel.addStretch().
        panel.addStretch()
        # Obtiene self.retry con button('Reconectar vista', self.controller.retry_view, tooltip='Reab…
        self.retry = button("Reconectar vista", self.controller.retry_view, tooltip="Reabre solo el consumidor; no reinicia la cámara.")
        # Agrega el control al diseño de la ventana mediante panel.addWidget con self.retry.
        panel.addWidget(self.retry)
        # Agrega el control al diseño de la ventana mediante splitter.addWidget con info.
        splitter.addWidget(info)
        # Ejecuta splitter.setStretchFactor(0, 1).
        splitter.setStretchFactor(0, 1)
        # Ejecuta splitter.setStretchFactor(1, 0).
        splitter.setStretchFactor(1, 0)
        # Ejecuta splitter.setSizes([750, 270]).
        splitter.setSizes([750, 270])
        # Agrega el control al diseño de la ventana mediante layout.addWidget con splitter, 1.
        layout.addWidget(splitter, 1)
        # Obtiene metrics con QHBoxLayout().
        metrics = QHBoxLayout()
        # Ejecuta metrics.setSpacing(10).
        metrics.setSpacing(10)
        # Obtiene self.capture_metric con MetricCard('FPS de captura', 'Promedio del productor calculado a par…
        self.capture_metric = MetricCard("FPS de captura", "Promedio del productor calculado a partir de sus timestamps.")
        # Obtiene self.consumer_metric con MetricCard('FPS de proceso', 'Ritmo de frames decodificados y proces…
        self.consumer_metric = MetricCard("FPS de proceso", "Ritmo de frames decodificados y procesados en la ventana reciente.")
        # Obtiene self.skipped_metric con MetricCard('Frames omitidos', 'Saltos de secuencia observados por es…
        self.skipped_metric = MetricCard("Frames omitidos", "Saltos de secuencia observados por este consumidor.")
        # Obtiene self.overwritten_metric con MetricCard('Sobrescrituras', 'Frames sobrescritos por el productor a…
        self.overwritten_metric = MetricCard("Sobrescrituras", "Frames sobrescritos por el productor antes de su consumo.")
        # Inicializa self.metric_cards con [self.capture_metric, self.consumer_metric, self.skipped_metric, self.overwritten…
        self.metric_cards = [self.capture_metric, self.consumer_metric, self.skipped_metric, self.overwritten_metric]
        # Recorre self.metric_cards y toma cada elemento como metric.
        for metric in self.metric_cards:
            # Agrega el control al diseño de la ventana mediante metrics.addWidget con metric, 1.
            metrics.addWidget(metric, 1)
        # Agrega una distribución de controles mediante layout.addLayout con metrics.
        layout.addLayout(metrics)
        # Obtiene self.notice con label('Lista. Pulsa Iniciar para conectar el motor.', 'muted').
        self.notice = label("Lista. Pulsa Iniciar para conectar el motor.", "muted")
        # Agrega el control al diseño de la ventana mediante layout.addWidget con self.notice.
        layout.addWidget(self.notice)
        # Agrega el control al diseño de la ventana mediante self.pages.addWidget con page.
        self.pages.addWidget(page)

    # Define _diagnostics_page sin parámetros de llamada.
    def _diagnostics_page(self):
        # Obtiene page con QWidget().
        page = QWidget()
        # Obtiene layout con QVBoxLayout(page).
        layout = QVBoxLayout(page)
        # Ejecuta layout.setContentsMargins(0, 0, 0, 0).
        layout.setContentsMargins(0, 0, 0, 0)
        # Agrega el control al diseño de la ventana mediante layout.addWidget con label('Diagnóstico de esta ejecución', 'heading').
        layout.addWidget(label("Diagnóstico de esta ejecución", "heading"))
        # Agrega el control al diseño de la ventana mediante layout.addWidget con label('Eventos del motor y de la vista. No contiene imágenes…
        layout.addWidget(label("Eventos del motor y de la vista. No contiene imágenes ni video; revisa los datos antes de compartirlo.", "muted"))
        # Obtiene row con QHBoxLayout().
        row = QHBoxLayout()
        # Ejecuta row.addStretch().
        row.addStretch()
        # Agrega el control al diseño de la ventana mediante row.addWidget con button('Guardar diagnóstico…', self._export_diagnostics).
        row.addWidget(button("Guardar diagnóstico…", self._export_diagnostics))
        # Agrega una distribución de controles mediante layout.addLayout con row.
        layout.addLayout(row)
        # Obtiene self.logs con QPlainTextEdit().
        self.logs = QPlainTextEdit()
        # Ejecuta self.logs.setReadOnly(True).
        self.logs.setReadOnly(True)
        # Ejecuta self.logs.document().setMaximumBlockCount(300).
        self.logs.document().setMaximumBlockCount(300)
        # Ejecuta self.logs.setAccessibleName("Registro de diagnóstico").
        self.logs.setAccessibleName("Registro de diagnóstico")
        # Agrega el control al diseño de la ventana mediante layout.addWidget con self.logs, 1.
        layout.addWidget(self.logs, 1)
        # Agrega el control al diseño de la ventana mediante self.pages.addWidget con page.
        self.pages.addWidget(page)

    # Define _settings_page sin parámetros de llamada.
    def _settings_page(self):
        # Obtiene page con QWidget().
        page = QWidget()
        # Obtiene layout con QVBoxLayout(page).
        layout = QVBoxLayout(page)
        # Agrega el control al diseño de la ventana mediante layout.addWidget con label('Ajustes de captura e historial', 'heading').
        layout.addWidget(label("Ajustes de captura e historial", "heading"))
        # Agrega el control al diseño de la ventana mediante layout.addWidget con label('Los cambios se aplican a la siguiente sesión. Puedes…
        layout.addWidget(label("Los cambios se aplican a la siguiente sesión. Puedes cambiar el filtro en directo desde Captura.", "muted"))
        # Obtiene form con QFormLayout().
        form = QFormLayout()
        # Obtiene self.profile con QComboBox().
        self.profile = QComboBox()
        # Ejecuta self.profile.setAccessibleName("Perfil de captura").
        self.profile.setAccessibleName("Perfil de captura")
        # Recorre PROFILES.items() y toma cada elemento como (name, (width, height, fps)).
        for name, (width, height, fps) in PROFILES.items():
            # Agrega una opción a la lista mediante self.profile.addItem con f'{width}×{height} · {fps} FPS', name.
            self.profile.addItem(f"{width}×{height} · {fps} FPS", name)
        # Ejecuta self.profile.setCurrentIndex(max(0, self.profile.findData(self.settings.value("capture/…
        self.profile.setCurrentIndex(max(0, self.profile.findData(self.settings.value("capture/profile", "720p30"))))
        # Obtiene self.slots (posiciones del anillo) con QSpinBox().
        self.slots = QSpinBox()
        # Establece los límites del control mediante self.slots.setRange con 2, 8.
        self.slots.setRange(2, 8)
        # Establece el valor del control o preferencia mediante self.slots.setValue con self.settings.value('capture/slots', 3, type=int).
        self.slots.setValue(self.settings.value("capture/slots", 3, type=int))
        # Obtiene self.interval con QSpinBox().
        self.interval = QSpinBox()
        # Establece los límites del control mediante self.interval.setRange con 1, 60.
        self.interval.setRange(1, 60)
        # Ejecuta self.interval.setSuffix(" s").
        self.interval.setSuffix(" s")
        # Establece el valor del control o preferencia mediante self.interval.setValue con self.settings.value('history/interval', 1, type=int).
        self.interval.setValue(self.settings.value("history/interval", 1, type=int))
        # Obtiene self.motion_area con QDoubleSpinBox().
        self.motion_area = QDoubleSpinBox()
        # Establece los límites del control mediante self.motion_area.setRange con 0.05, 10.
        self.motion_area.setRange(0.05, 10)
        # Ejecuta self.motion_area.setSingleStep(0.05).
        self.motion_area.setSingleStep(0.05)
        # Ejecuta self.motion_area.setSuffix(" %").
        self.motion_area.setSuffix(" %")
        # Establece el valor del control o preferencia mediante self.motion_area.setValue con self.settings.value('vision/motion_area', 0.25, type=float).
        self.motion_area.setValue(self.settings.value("vision/motion_area", 0.25, type=float))
        # Recorre (('Formato solicitado', self.profile), ('Slots del búfer', self.slots), ('Intervalo de… y toma cada elemento como (title, field).
        for title, field in (("Formato solicitado", self.profile), ("Slots del búfer", self.slots),
                             # Inicia ("Intervalo de muestras", self.interval), ("Área mínima de movimiento", self.motion_are… con form.addRow(title, field).
                             ("Intervalo de muestras", self.interval), ("Área mínima de movimiento", self.motion_area)):
            # Ejecuta form.addRow(title, field).
            form.addRow(title, field)
        # Agrega una distribución de controles mediante layout.addLayout con form.
        layout.addLayout(form)
        # Agrega el control al diseño de la ventana mediante layout.addWidget con label('La cámara negocia el formato según sus capacidades. U…
        layout.addWidget(label("La cámara negocia el formato según sus capacidades. Un área mínima mayor filtra cambios pequeños.", "muted"))
        # Ejecuta layout.addStretch().
        layout.addStretch()
        # Conecta el evento con su manejador mediante self.profile.currentIndexChanged.connect con self._save_preferences.
        self.profile.currentIndexChanged.connect(self._save_preferences)
        # Conecta el evento con su manejador mediante self.slots.valueChanged.connect con self._save_preferences.
        self.slots.valueChanged.connect(self._save_preferences)
        # Conecta el evento con su manejador mediante self.interval.valueChanged.connect con self._save_preferences.
        self.interval.valueChanged.connect(self._save_preferences)
        # Conecta el evento con su manejador mediante self.motion_area.valueChanged.connect con self._save_preferences.
        self.motion_area.valueChanged.connect(self._save_preferences)
        # Agrega el control al diseño de la ventana mediante self.pages.addWidget con page.
        self.pages.addWidget(page)

    # Define _save_preferences sin parámetros de llamada.
    def _save_preferences(self):
        # Establece el valor del control o preferencia mediante self.settings.setValue con 'capture/profile', self.profile.currentData().
        self.settings.setValue("capture/profile", self.profile.currentData())
        # Establece el valor del control o preferencia mediante self.settings.setValue con 'capture/slots', self.slots.value().
        self.settings.setValue("capture/slots", self.slots.value())
        # Establece el valor del control o preferencia mediante self.settings.setValue con 'history/interval', self.interval.value().
        self.settings.setValue("history/interval", self.interval.value())
        # Establece el valor del control o preferencia mediante self.settings.setValue con 'vision/motion_area', self.motion_area.value().
        self.settings.setValue("vision/motion_area", self.motion_area.value())

    # Define _mode_changed sin parámetros de llamada.
    def _mode_changed(self):
        # Obtiene mode (modo de procesamiento) con self.mode.currentData().
        mode = self.mode.currentData()
        # Actualiza el filtro de procesamiento mediante self.controller.set_mode con mode.
        self.controller.set_mode(mode)
        # Establece el valor del control o preferencia mediante self.settings.setValue con 'vision/mode', mode.
        self.settings.setValue("vision/mode", mode)
        # Ejecuta self._log("success", "Modo de visión · " + MODES[mode]).
        self._log("success", "Modo de visión · " + MODES[mode])

    # Define _about_page sin parámetros de llamada.
    def _about_page(self):
        # Obtiene page con QScrollArea().
        page = QScrollArea()
        # Ejecuta page.setWidgetResizable(True).
        page.setWidgetResizable(True)
        # Obtiene content con QWidget().
        content = QWidget()
        # Obtiene layout con QVBoxLayout(content).
        layout = QVBoxLayout(content)
        # Ejecuta layout.setContentsMargins(12, 12, 22, 20).
        layout.setContentsMargins(12, 12, 22, 20)
        # Ejecuta layout.setSpacing(18).
        layout.setSpacing(18)
        # Agrega el control al diseño de la ventana mediante layout.addWidget con label('ORVIX · Captura y visión en tiempo real', 'heading').
        layout.addWidget(label("ORVIX · Captura y visión en tiempo real", "heading"))
        # Agrega el control al diseño de la ventana mediante layout.addWidget con label('Motor nativo C++ + protocolo IPC v1 + consumidor Pyth…
        layout.addWidget(label("Motor nativo C++ + protocolo IPC v1 + consumidor Python / NumPy / OpenCV + interfaz Qt.", "section"))
        # Agrega el control al diseño de la ventana mediante layout.addWidget con label('Flujo de trabajo', 'section').
        layout.addWidget(label("Flujo de trabajo", "section"))
        # Agrega el control al diseño de la ventana mediante layout.addWidget con label('• Enumerar y seleccionar cámara; iniciar y detener si…
        layout.addWidget(label("• Enumerar y seleccionar cámara; iniciar y detener sin terminal.\n• Fuente sintética para demostrar el puente sin webcam.\n• RAW, grises, bordes y detección de movimiento en directo.\n• Metadatos, FPS, saltos de secuencia y utilización del anillo.\n• Errores visibles, reconexión del consumidor y diagnóstico exportable.", "muted"))
        # Agrega el control al diseño de la ventana mediante layout.addWidget con label('Historial guarda muestras y diagnósticos en SQLite. A…
        layout.addWidget(label("Historial guarda muestras y diagnósticos en SQLite. Ajustes permite cambiar el perfil, el búfer y la sensibilidad del detector.", "muted"))
        # Agrega el control al diseño de la ventana mediante layout.addWidget con label('Atajos y privacidad', 'section').
        layout.addWidget(label("Atajos y privacidad", "section"))
        # Agrega el control al diseño de la ventana mediante layout.addWidget con label('Ctrl+Enter: iniciar · Esc: detener · F11: pantalla co…
        layout.addWidget(label("Ctrl+Enter: iniciar · Esc: detener · F11: pantalla completa · Ctrl+0: ajustar.\nRueda: zoom · Arrastrar: mover la imagen ampliada · Doble clic: ajustar.\nLos frames viven en memoria. Los logs técnicos se guardan localmente; no se suben a ningún servicio.", "muted"))
        # Agrega el control al diseño de la ventana mediante layout.addWidget con label('Equipo', 'section').
        layout.addWidget(label("Equipo", "section"))
        # Agrega el control al diseño de la ventana mediante layout.addWidget con label('Andre Alexander Alvarado Lira\nJose Eduardo Alvarado…
        layout.addWidget(label("Andre Alexander Alvarado Lira\nJose Eduardo Alvarado Mejia\nJavier Gomez Correa", "muted"))
        # Agrega el control al diseño de la ventana mediante layout.addWidget con label('Versión ' + __version__ + ' · Paquete para Windows x6…
        layout.addWidget(label("Versión " + __version__ + " · Paquete para Windows x64.\nEl núcleo conserva los backends nativos multiplataforma.\nLa licencia ORVIX es MIT; Qt/PySide se distribuyen con sus licencias correspondientes.", "muted"))
        # Ejecuta layout.addStretch().
        layout.addStretch()
        # Ejecuta page.setWidget(content).
        page.setWidget(content)
        # Agrega el control al diseño de la ventana mediante self.pages.addWidget con page.
        self.pages.addWidget(page)

    # Define _devices_changed con el parámetro devices.
    def _devices_changed(self, devices):
        # Obtiene selected con self.source.currentData().
        selected = self.source.currentData()
        # Vacía el contenedor mediante self.source.clear.
        self.source.clear()
        # Agrega una opción a la lista mediante self.source.addItem con 'Demo sintética · sin cámara', None.
        self.source.addItem("Demo sintética · sin cámara", None)
        # Recorre devices y toma cada elemento como item.
        for item in devices:
            # Agrega una opción a la lista mediante self.source.addItem con '{} · {}'.format(item.name, item.backend), item.index.
            self.source.addItem("{} · {}".format(item.name, item.backend), item.index)
            # Ejecuta self.source.setItemData(self.source.count() - 1, item, Qt.UserRole + 1).
            self.source.setItemData(self.source.count() - 1, item, Qt.UserRole + 1)
        # Entra en esta rama si selected is not None.
        if selected is not None:
            # Obtiene idx con self.source.findData(selected).
            idx = self.source.findData(selected)
            # Entra en esta rama si idx >= 0.
            if idx >= 0:
                # Ejecuta self.source.setCurrentIndex(idx).
                self.source.setCurrentIndex(idx)
        # Cambia si el control admite interacción mediante self.refresh.setEnabled con not self.controller.active.
        self.refresh.setEnabled(not self.controller.active)
        # Cambia si el control admite interacción mediante self.start.setEnabled con not self.controller.active.
        self.start.setEnabled(not self.controller.active)

    # Define _start sin parámetros de llamada.
    def _start(self):
        # Entra en esta rama si self.controller.active.
        if self.controller.active:
            # Sale de la función sin devolver un valor.
            return
        # Entra en esta rama si self.controller.enumerator.state().name != 'NotRunning'.
        if self.controller.enumerator.state().name != "NotRunning":
            # Actualiza el texto visible del control mediante self.notice.setText con 'Espera a que termine la búsqueda de cámaras.'.
            self.notice.setText("Espera a que termine la búsqueda de cámaras.")
            # Sale de la función sin devolver un valor.
            return
        # Guarda en self._started_at el resultado de time.monotonic, que lee un reloj monotónico.
        self._started_at = time.monotonic()
        # Establece self._last_metrics a partir de None.
        self._last_metrics = None
        # Establece self._sample_after a partir de 0.0.
        self._sample_after = 0.0
        # Obtiene self.controller.profile con self.profile.currentData().
        self.controller.profile = self.profile.currentData()
        # Obtiene self.controller.slots (posiciones del anillo) con self.slots.value().
        self.controller.slots = self.slots.value()
        # Obtiene self.controller.motion_area con self.motion_area.value().
        self.controller.motion_area = self.motion_area.value()
        # Obtiene self._sample_interval con self.interval.value().
        self._sample_interval = self.interval.value()
        # Obtiene index con self.source.currentIndex().
        index = self.source.currentIndex()
        # Obtiene choice con self.source.itemData(index, Qt.UserRole + 1).
        choice = self.source.itemData(index, Qt.UserRole + 1)
        # Establece self._session_id (identificador de sesión) a partir de uuid.uuid4().hex if self.history else None.
        self._session_id = uuid.uuid4().hex if self.history else None
        # Entra en esta rama si self._session_id and (not self._record(self.history.begin, self._session_id, 'camera' i…
        if self._session_id and not self._record(
            # Aporta self.history.begin, self._session_id, "camera" if choice else "synthetic" como elemento de la lista.
            self.history.begin, self._session_id, "camera" if choice else "synthetic",
            # Aporta choice.name if choice else "Demo sintética", choice.backend if choice else "Sinté… como elemento de la lista.
            choice.name if choice else "Demo sintética", choice.backend if choice else "Sintético",
            # Aporta self.controller.mode, self.controller.profile, self.controller.slots como elemento de la lista.
            self.controller.mode, self.controller.profile, self.controller.slots,
            # Aporta self._sample_interval, self.controller.motion_area como elemento de la lista.
            self._sample_interval, self.controller.motion_area,
        # Completa la expresión y encadena :.
        ):
            # Establece self._session_id (identificador de sesión) a partir de None.
            self._session_id = None
        # Establece self.history_page.live_session a partir de self._session_id.
        self.history_page.live_session = self._session_id
        # Actualiza el texto visible del control mediante self.vision_info.setText con 'Latencia de procesamiento · —\nDetecciones en 60 s · —'.
        self.vision_info.setText("Latencia de procesamiento · —\nDetecciones en 60 s · —")
        # Recorre self.metric_cards y toma cada elemento como metric.
        for metric in self.metric_cards:
            # Ejecuta metric.set_value("—").
            metric.set_value("—")
        # Establece el valor del control o preferencia mediante self.buffer_bar.setValue con 0.
        self.buffer_bar.setValue(0)
        # Vacía el contenedor mediante self.preview.clear con 'Conectando…', 'El motor prepara la fuente seleccionada.'.
        self.preview.clear("Conectando…", "El motor prepara la fuente seleccionada.")
        # Ejecuta self.controller.start(self.source.currentData()).
        self.controller.start(self.source.currentData())

    # Define _stop sin parámetros de llamada.
    def _stop(self):
        # Ejecuta self.controller.stop().
        self.controller.stop()

    # Define _state_changed con los parámetros state, text.
    def _state_changed(self, state, text):
        # Ejecuta self.notice.setStyleSheet("").
        self.notice.setStyleSheet("")
        # Establece self._state_text a partir de text.
        self._state_text = text
        # Actualiza el texto visible del control mediante self.status.setText con {'idle': '○ Lista', 'starting': '◌ Conectando', 'running': '…
        self.status.setText({"idle": "○  Lista", "starting": "◌  Conectando", "running": "●  En directo",
                             # Asocia la clave "stopping" con "◌ Deteniendo", "error": "! Atención"}[state]).
                             "stopping": "◌  Deteniendo", "error": "!  Atención"}[state])
        # Ejecuta self.status.setProperty("state", state).
        self.status.setProperty("state", state)
        # Ejecuta self.status.style().unpolish(self.status).
        self.status.style().unpolish(self.status)
        # Ejecuta self.status.style().polish(self.status).
        self.status.style().polish(self.status)
        # Actualiza el texto visible del control mediante self.notice.setText con text.
        self.notice.setText(text)
        # Establece busy a partir de self.controller.active or state in ('starting', 'stopping').
        busy = self.controller.active or state in ("starting", "stopping")
        # Cambia si el control admite interacción mediante self.start.setEnabled con not busy.
        self.start.setEnabled(not busy)
        # Cambia si el control admite interacción mediante self.source.setEnabled con not busy.
        self.source.setEnabled(not busy)
        # Cambia si el control admite interacción mediante self.refresh.setEnabled con not busy.
        self.refresh.setEnabled(not busy)
        # Cambia si el control admite interacción mediante self.stop.setEnabled con busy and state != 'stopping'.
        self.stop.setEnabled(busy and state != "stopping")
        # Cambia si el control admite interacción mediante self.retry.setEnabled con state == 'error' and self.controller.process.state().name ==…
        self.retry.setEnabled(state == "error" and self.controller.process.state().name == "Running")
        # Entra en esta rama si state == 'idle'.
        if state == "idle":
            # Ejecuta self.pipeline.setStyleSheet("").
            self.pipeline.setStyleSheet("")
            # Vacía el contenedor mediante self.preview.clear con 'Captura detenida' if self._started_at else 'Tu captura empi…
            self.preview.clear("Captura detenida" if self._started_at else "Tu captura empieza aquí",
                               # Incorpora "Pulsa Iniciar cuando estés listo.\nLa cámara está liberada.") en esta operación.
                               "Pulsa Iniciar cuando estés listo.\nLa cámara está liberada.")
            # Actualiza el texto visible del control mediante self.pipeline.setText con '○ Motor detenido\n\n○ Bridge liberado\n\n○ Consumidor deten…
            self.pipeline.setText("○  Motor detenido\n\n○  Bridge liberado\n\n○  Consumidor detenido")
            # Establece el valor del control o preferencia mediante self.buffer_bar.setValue con 0.
            self.buffer_bar.setValue(0)
            # Actualiza el texto visible del control mediante self.buffer_info.setText con 'Puente liberado · sin ocupación activa'.
            self.buffer_info.setText("Puente liberado · sin ocupación activa")
            # Entra en esta rama si self._last_metrics.
            if self._last_metrics:
                # Establece data a partir de self._last_metrics.
                data = self._last_metrics
                # Actualiza el texto visible del control mediante self.session_info.setText con 'Última captura detenida\n\n{}×{} · {}\n\nLas tarjetas muest…
                self.session_info.setText("Última captura detenida\n\n{}×{} · {}\n\nLas tarjetas muestran las últimas mediciones, no una captura activa.".format(
                    # Incorpora data["width"], data["height"], data["format"])) en esta operación.
                    data["width"], data["height"], data["format"]))
                # Actualiza el texto visible del control mediante self.notice.setText con text + ' · últimas mediciones de la captura detenida'.
                self.notice.setText(text + " · últimas mediciones de la captura detenida")
        # Entra en esta rama si state == 'starting'.
        elif state == "starting":
            # Actualiza el texto visible del control mediante self.pipeline.setText con '◌ Preparando motor\n\n◌ Esperando Bridge\n\n○ Consumidor pe…
            self.pipeline.setText("◌  Preparando motor\n\n◌  Esperando Bridge\n\n○  Consumidor pendiente")
        # Entra en esta rama si state == 'error'.
        elif state == "error":
            # Ejecuta self.pipeline.setStyleSheet("color: #ffb6be;").
            self.pipeline.setStyleSheet("color: #ffb6be;")
            # Vacía el contenedor mediante self.preview.clear con 'La captura requiere atención', 'Consulta Diagnóstico y rein…
            self.preview.clear("La captura requiere atención", "Consulta Diagnóstico y reintenta.\nNo se muestra un frame antiguo como si fuera directo.")
            # Actualiza el texto visible del control mediante self.pipeline.setText con '! Revisa el diagnóstico\n\n○ Vista sin frames'.
            self.pipeline.setText("!  Revisa el diagnóstico\n\n○  Vista sin frames")

    # Define _connected con el parámetro header.
    def _connected(self, header):
        # Entra en esta rama si self._started_at is None.
        if self._started_at is None:
            # Guarda en self._started_at el resultado de time.monotonic, que lee un reloj monotónico.
            self._started_at = time.monotonic()
        # Entra en esta rama si self._session_id.
        if self._session_id:
            # Ejecuta self._record(self.history.connected, self._session_id, header).
            self._record(self.history.connected, self._session_id, header)
        # Actualiza el texto visible del control mediante self.pipeline.setText con '● Motor nativo activo\n\n● Bridge conectado\n\n● Procesamie…
        self.pipeline.setText("●  Motor nativo activo\n\n●  Bridge conectado\n\n●  Procesamiento activo")
        # Ejecuta self.pipeline.setStyleSheet("color: #66e8b3;").
        self.pipeline.setStyleSheet("color: #66e8b3;")
        # Actualiza el texto visible del control mediante self.session_info.setText con '{}×{} · {}\nPID nativo: {}\n\nObjetivo: {:.0f} FPS\nFuente:…
        self.session_info.setText("{}×{} · {}\nPID nativo: {}\n\nObjetivo: {:.0f} FPS\nFuente: {}".format(
            # Aporta header.width, header.height, header.pixel_format, header.producer_pid como elemento de la lista.
            header.width, header.height, header.pixel_format, header.producer_pid,
            # Ejecuta header.fps_numerator / max(1, header.fps_denominator), self.source.currentText())).
            header.fps_numerator / max(1, header.fps_denominator), self.source.currentText()))

    # Define _render_latest sin parámetros de llamada.
    def _render_latest(self):
        # Obtiene value con self.controller.mailbox.take().
        value = self.controller.mailbox.take()
        # Entra en esta rama si value is None or self.controller.state != 'running'.
        if value is None or self.controller.state != "running":
            # Sale de la función sin devolver un valor.
            return
        # Establece (image, data) a partir de value.
        image, data = value
        # Establece self._last_metrics a partir de data.
        self._last_metrics = data
        # Entra en esta rama si self.pages.currentIndex() == 0.
        if self.pages.currentIndex() == 0:
            # Ejecuta self.preview.set_image(image).
            self.preview.set_image(image)
        # Ejecuta self.capture_metric.set_value("{:.1f}".format(data["capture_fps"])).
        self.capture_metric.set_value("{:.1f}".format(data["capture_fps"]))
        # Ejecuta self.consumer_metric.set_value("{:.1f}".format(data["consumer_fps"])).
        self.consumer_metric.set_value("{:.1f}".format(data["consumer_fps"]))
        # Ejecuta self.skipped_metric.set_value(str(data["skipped"])).
        self.skipped_metric.set_value(str(data["skipped"]))
        # Ejecuta self.overwritten_metric.set_value(str(data["overwrites"])).
        self.overwritten_metric.set_value(str(data["overwrites"]))
        # Establece el valor del control o preferencia mediante self.buffer_bar.setValue con round(data['buffer']).
        self.buffer_bar.setValue(round(data["buffer"]))
        # Actualiza el texto visible del control mediante self.buffer_info.setText con '{:.0f}% · {} slots'.format(data['buffer'], data['slots']).
        self.buffer_info.setText("{:.0f}% · {} slots".format(data["buffer"], data["slots"]))
        # Actualiza el texto visible del control mediante self.vision_info.setText con f"{data['latency_ms']:.2f} ms de proceso\n{data['detections_…
        self.vision_info.setText(
            # Aporta el texto f'{data["latency_ms"]:.2f} ms de proceso\n{data["detections_min"]} detecciones en 60 s' a la expresión.
            f'{data["latency_ms"]:.2f} ms de proceso\n{data["detections_min"]} detecciones en 60 s'
        )
        # Calcula los segundos transcurridos desde el inicio con un reloj monotónico.
        elapsed = time.monotonic() - self._started_at
        # Guarda una muestra solo si la sesión está activa y venció el intervalo.
        if self._session_id and elapsed >= self._sample_after:
            # Solicita el guardado de las métricas actuales en la sesión activa.
            self._record(self.history.sample, self._session_id, elapsed, data)
            # Programa el siguiente guardado sumando el intervalo configurado.
            self._sample_after = elapsed + self._sample_interval
        # Actualiza el texto visible del control mediante self.metadata.setText con 'Frame #{} · {}×{} · {} · {:,} bytes · timestamp {} ×100 ns'…
        self.metadata.setText("Frame #{} · {}×{} · {} · {:,} bytes · timestamp {} ×100 ns".format(
            # Incorpora data["sequence"], data["width"], data["height"], data["format"], data["bytes"], data["t… en esta operación.
            data["sequence"], data["width"], data["height"], data["format"], data["bytes"], data["timestamp"]))
        # Actualiza el texto visible del control mediante self.notice.setText con 'Captura en directo · {:.0f} s · sin grabación · amplía con…
        self.notice.setText("Captura en directo · {:.0f} s · sin grabación · amplía con la rueda y arrastra para moverte".format(time.monotonic() - self._started_at))

    # Define _record con el parámetro operation.
    def _record(self, operation, *args):
        # Ejecuta el bloque con manejo de excepciones.
        try:
            # Ejecuta operation(*args).
            operation(*args)
        # Maneja sqlite3.Error.
        except sqlite3.Error as error:
            # Entra en esta rama si self._history_problem != str(error).
            if self._history_problem != str(error):
                # Obtiene self._history_problem con str(error).
                self._history_problem = str(error)
                # Ejecuta self.logs.appendPlainText("No se pudo guardar el historial. " + str(error)).
                self.logs.appendPlainText("No se pudo guardar el historial. " + str(error))
            # Devuelve False.
            return False
        # Devuelve True.
        return True

    # Define _log con los parámetros level, text.
    def _log(self, level, text):
        # Entra en esta rama si self._session_id and level in ('success', 'warning', 'error').
        if self._session_id and level in ("success", "warning", "error"):
            # Ejecuta self._record(self.history.event, self._session_id,.
            self._record(self.history.event, self._session_id,
                         # Lee un reloj monotónico mediante time.monotonic.
                         time.monotonic() - self._started_at, level, text)
        # Ejecuta self.logs.appendPlainText("{} {:7} {}".format(datetime.now().strftime("%H:%M:%S"), leve…
        self.logs.appendPlainText("{}  {:7}  {}".format(datetime.now().strftime("%H:%M:%S"), level.upper(), text))
        # Entra en esta rama si level in ('error', 'warning').
        if level in ("error", "warning"):
            # Actualiza el texto visible del control mediante self.notice.setText con text.
            self.notice.setText(text)
            # Comprueba la igualdad indicada por self.notice.setStyleSheet("color: #ffb6be;" if level == "error" else "color: #edcd97;").
            self.notice.setStyleSheet("color: #ffb6be;" if level == "error" else "color: #edcd97;")

    # Define _export_diagnostics sin parámetros de llamada.
    def _export_diagnostics(self):
        # Obtiene (destination, _filter) con QFileDialog.getSaveFileName(self, 'Guardar diagnóstico', 'orvix-diag…
        destination, _filter = QFileDialog.getSaveFileName(self, "Guardar diagnóstico", "orvix-diagnostico.txt", "Texto (*.txt)")
        # Entra en esta rama si destination.
        if destination:
            # Ejecuta el bloque con manejo de excepciones.
            try:
                # Ejecuta Path(destination).write_text(self.logs.toPlainText() + "\n", encoding="utf-8").
                Path(destination).write_text(self.logs.toPlainText() + "\n", encoding="utf-8")
                # Ejecuta self._log("success", "Diagnóstico guardado en el archivo elegido.").
                self._log("success", "Diagnóstico guardado en el archivo elegido.")
            # Maneja OSError.
            except OSError as error:
                # Ejecuta QMessageBox.warning(self, "No se pudo guardar", str(error)).
                QMessageBox.warning(self, "No se pudo guardar", str(error))

    # Define _fullscreen sin parámetros de llamada.
    def _fullscreen(self):
        # Ejecuta self.showNormal() if self.isFullScreen() else self.showFullScreen().
        self.showNormal() if self.isFullScreen() else self.showFullScreen()

    # Define _quiescent sin parámetros de llamada.
    def _quiescent(self):
        # Entra en esta rama si self._session_id.
        if self._session_id:
            # Calcula los segundos transcurridos desde el inicio con un reloj monotónico.
            elapsed = time.monotonic() - self._started_at
            # Entra en esta rama si self._last_metrics.
            if self._last_metrics:
                # Intenta guardar las últimas métricas antes de cerrar la sesión.
                self._record(self.history.sample, self._session_id, elapsed, self._last_metrics)
            # Establece state (estado) a partir de 'failed' if self.controller.state == 'error' else 'stopped'.
            state = "failed" if self.controller.state == "error" else "stopped"
            # Entra en esta rama si self.controller.forced_stop.
            if self.controller.forced_stop:
                # Establece state (estado) a partir de 'forced'.
                state = "forced"
            # Registra la duración y el estado final de la sesión.
            self._record(self.history.finish, self._session_id, elapsed, state)
            # Establece self._session_id (identificador de sesión) a partir de None.
            self._session_id = None
            # Establece self.history_page.live_session a partir de None.
            self.history_page.live_session = None
            # Entra en esta rama si self.pages.currentWidget() is self.history_page.
            if self.pages.currentWidget() is self.history_page:
                # Ejecuta self.history_page.refresh().
                self.history_page.refresh()
        # Entra en esta rama si self._close_when_idle.
        if self._close_when_idle:
            # Ejecuta QTimer.singleShot(0, self.close).
            QTimer.singleShot(0, self.close)

    # Define closeEvent con el parámetro event.
    def closeEvent(self, event: QCloseEvent):
        # Entra en esta rama si self.controller.active or self.controller.enumerator.state() != QProcess.NotRunning.
        if self.controller.active or self.controller.enumerator.state() != QProcess.NotRunning:
            # Entra en esta rama si not self._close_when_idle.
            if not self._close_when_idle:
                # Obtiene answer con QMessageBox.question(self, 'Cerrar ORVIX', '¿Detener la captura, lib…
                answer = QMessageBox.question(self, "Cerrar ORVIX", "¿Detener la captura, liberar la cámara y cerrar?",
                                              # Incorpora QMessageBox.Yes | QMessageBox.Cancel, QMessageBox.Cancel) en esta operación.
                                              QMessageBox.Yes | QMessageBox.Cancel, QMessageBox.Cancel)
                # Entra en esta rama si answer != QMessageBox.Yes.
                if answer != QMessageBox.Yes:
                    # Ejecuta event.ignore().
                    event.ignore()
                    # Sale de la función sin devolver un valor.
                    return
            # Establece self._close_when_idle a partir de True.
            self._close_when_idle = True
            # Ejecuta event.ignore().
            event.ignore()
            # Ejecuta self.controller.shutdown().
            self.controller.shutdown()
            # Sale de la función sin devolver un valor.
            return
        # Establece self._close_when_idle a partir de False.
        self._close_when_idle = False
        # Ejecuta self.controller.shutdown().
        self.controller.shutdown()
        # Establece el valor del control o preferencia mediante self.settings.setValue con 'window/geometry', self.saveGeometry().
        self.settings.setValue("window/geometry", self.saveGeometry())
        # Ejecuta self.settings.sync().
        self.settings.sync()
        # Ejecuta self.timer.stop().
        self.timer.stop()
        # Entra en esta rama si self.history.
        if self.history:
            # Cierra el recurso mediante self.history.close.
            self.history.close()
            # Establece self.history (historial SQLite) a partir de None.
            self.history = None
            # Establece self.history_page.history (historial SQLite) a partir de None.
            self.history_page.history = None
        # Ejecuta event.accept().
        event.accept()


# Define main con el parámetro argv.
def main(argv=None):
    # Obtiene parser con argparse.ArgumentParser(description='ORVIX desktop').
    parser = argparse.ArgumentParser(description="ORVIX desktop")
    # Ejecuta parser.add_argument("--engine", help="Path to a native ORVIX capture executable").
    parser.add_argument("--engine", help="Path to a native ORVIX capture executable")
    # Ejecuta parser.add_argument("--data-dir", type=Path, help="Keep settings, logs and SQLite in th…
    parser.add_argument("--data-dir", type=Path, help="Keep settings, logs and SQLite in this directory")
    # Ejecuta parser.add_argument("--demo-seconds", type=int, help="Run the synthetic source for 2–60…
    parser.add_argument("--demo-seconds", type=int, help="Run the synthetic source for 2–60 seconds and exit")
    # Obtiene args con parser.parse_args(argv).
    args = parser.parse_args(argv)
    # Entra en esta rama si args.demo_seconds is not None and (not 2 <= args.demo_seconds <= 60).
    if args.demo_seconds is not None and not 2 <= args.demo_seconds <= 60:
        # Ejecuta parser.error('--demo-seconds must be between 2 and 60').
        parser.error('--demo-seconds must be between 2 and 60')
    # Establece app a partir de QApplication.instance() or QApplication(sys.argv[:1]).
    app = QApplication.instance() or QApplication(sys.argv[:1])
    # Ejecuta app.setOrganizationName("ORVIX").
    app.setOrganizationName("ORVIX")
    # Ejecuta app.setApplicationName("ORVIX").
    app.setApplicationName("ORVIX")
    # Ejecuta app.setApplicationVersion(__version__).
    app.setApplicationVersion(__version__)
    # Ejecuta app.setStyle("Fusion").
    app.setStyle("Fusion")
    # Ejecuta el bloque con manejo de excepciones.
    try:
        # Establece settings (preferencias de la aplicación) a partir de QSettings(str(args.data_dir / 'orvix.ini'), QSettings.IniFormat) if args.data_dir…
        settings = QSettings(str(args.data_dir / "orvix.ini"), QSettings.IniFormat) if args.data_dir else None
        # Obtiene window con MainWindow(find_engine(args.engine), args.data_dir, settings).
        window = MainWindow(find_engine(args.engine), args.data_dir, settings)
    # Maneja (FileNotFoundError, OSError).
    except (FileNotFoundError, OSError) as error:
        # Ejecuta QMessageBox.critical(None, "No se pudo iniciar ORVIX", str(error)).
        QMessageBox.critical(None, "No se pudo iniciar ORVIX", str(error))
        # Devuelve 2.
        return 2
    # Ejecuta window.show().
    window.show()
    # Entra en esta rama si args.demo_seconds is None.
    if args.demo_seconds is None:
        # Devuelve app.exec().
        return app.exec()
    # Start before camera enumeration. This entry point always uses the synthetic source.
    # Ejecuta window.mode.setCurrentIndex(window.mode.findData("motion")).
    window.mode.setCurrentIndex(window.mode.findData("motion"))
    # Ejecuta window._start().
    window._start()
    # Ejecuta QTimer.singleShot(args.demo_seconds * 1000, window._stop).
    QTimer.singleShot(args.demo_seconds * 1000, window._stop)
    # Conecta el evento con su manejador mediante window.controller.quiescent.connect con window.close.
    window.controller.quiescent.connect(window.close)
    # Obtiene exit_code con app.exec().
    exit_code = app.exec()
    # Entra en esta rama si not window._last_metrics or window.controller.state == 'error' or window.controller.for…
    if not window._last_metrics or window.controller.state == "error" or window.controller.forced_stop:
        # Devuelve 2.
        return 2
    # Devuelve exit_code.
    return exit_code


# Entra en esta rama si __name__ == '__main__'.
if __name__ == "__main__":
    # Interrumpe la operación con SystemExit(main()).
    raise SystemExit(main())
