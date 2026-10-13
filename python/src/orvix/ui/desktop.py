"""ORVIX Avance 2 desktop: RAW video and native-bridge diagnostics only."""

from __future__ import annotations

import argparse
import sys
import time
from datetime import datetime
from pathlib import Path

from PySide6.QtCore import QProcess, QSettings, QTimer, Qt
from PySide6.QtGui import QCloseEvent, QIcon, QKeySequence, QShortcut
from PySide6.QtWidgets import (
    QApplication, QButtonGroup, QComboBox, QFileDialog, QFrame, QHBoxLayout,
    QMainWindow, QMessageBox, QPlainTextEdit, QProgressBar, QPushButton,
    QScrollArea, QSizePolicy, QSplitter, QStackedWidget, QVBoxLayout, QWidget,
)

from orvix.ui.capture_controller import CaptureController, find_engine
from orvix.ui.widgets import MetricCard, Preview, STYLE, card, label


def asset_path(name):
    return Path(__file__).parent / "assets" / name


def button(text, callback=None, name=None, tooltip=None):
    widget = QPushButton(text)
    if name:
        widget.setObjectName(name)
    if tooltip:
        widget.setToolTip(tooltip)
    if callback:
        widget.clicked.connect(callback)
    return widget


class MainWindow(QMainWindow):
    def __init__(self, engine, work_dir=None):
        super().__init__()
        self.setWindowTitle("ORVIX · Avance 2")
        self.setWindowIcon(QIcon(str(asset_path("orvix.svg"))))
        self.setMinimumSize(1024, 680)
        self.resize(1320, 860)
        self.settings = QSettings()
        self.controller = CaptureController(engine, self, work_dir)
        self._close_when_idle = False
        self._started_at = None
        self._last_metrics = None
        self._state_text = "Lista para capturar"
        self._build()
        self.setStyleSheet(STYLE)
        geometry = self.settings.value("window/geometry")
        if geometry:
            self.restoreGeometry(geometry)
        self.controller.state_changed.connect(self._state_changed)
        self.controller.devices_changed.connect(self._devices_changed)
        self.controller.diagnostics.connect(self._log)
        self.controller.connected.connect(self._connected)
        self.controller.quiescent.connect(self._quiescent)
        self.timer = QTimer(self)
        self.timer.timeout.connect(self._render_latest)
        self.timer.start(33)
        self._shortcuts = []
        for key, callback in (("Ctrl+Return", self._start), ("Escape", self._stop),
                              ("F11", self._fullscreen), ("Ctrl+0", self.preview.fit)):
            shortcut = QShortcut(QKeySequence(key), self)
            shortcut.activated.connect(callback)
            self._shortcuts.append(shortcut)
        self._state_changed("idle", "Lista para capturar")
        self._log("info", "Avance 2 · Captura nativa + Bridge + visualización RAW. No se guardan imágenes ni video.")
        QTimer.singleShot(100, self.controller.refresh_devices)

    def _build(self):
        shell = QWidget()
        shell.setObjectName("shell")
        self.setCentralWidget(shell)
        outer = QHBoxLayout(shell)
        outer.setContentsMargins(0, 0, 0, 0)
        outer.setSpacing(0)
        side = QFrame()
        side.setObjectName("sidebar")
        side.setFixedWidth(190)
        nav = QVBoxLayout(side)
        nav.setContentsMargins(18, 28, 18, 24)
        nav.setSpacing(10)
        brand = label("◉  ORVIX", "heading")
        brand.setStyleSheet("color: #c1a5ff; font-size: 25px; letter-spacing: 2px;")
        nav.addWidget(brand)
        nav.addWidget(label("CAPTURE / BRIDGE", "muted"))
        nav.addSpacing(34)
        self.pages = QStackedWidget()
        self.nav_group = QButtonGroup(self)
        for idx, title in enumerate(("◉   Captura RAW", "≡   Diagnóstico", "ⓘ   Sobre el avance")):
            item = button(title, name="nav")
            item.setCheckable(True)
            self.nav_group.addButton(item, idx)
            item.clicked.connect(lambda _checked=False, i=idx: self.pages.setCurrentIndex(i))
            nav.addWidget(item)
            if idx == 0:
                item.setChecked(True)
        nav.addStretch()
        note = label("SOLO AVANCE 2\n\nFrames efímeros.\nSin grabación automática.", "muted")
        nav.addWidget(note)
        nav.addSpacing(12)
        nav.addWidget(label("0.2.1  ·  Windows x64", "muted"))
        outer.addWidget(side)
        body = QVBoxLayout()
        body.setContentsMargins(26, 24, 26, 20)
        body.setSpacing(18)
        header = QHBoxLayout()
        titles = QVBoxLayout()
        titles.setSpacing(4)
        titles.addWidget(label("Tu cámara. Bajo control.", "heading"))
        titles.addWidget(label("Captura nativa, memoria compartida y una vista clara del proceso.", "muted"))
        header.addLayout(titles, 1)
        self.status = label("Lista para capturar")
        self.status.setObjectName("status")
        header.addWidget(self.status, 0, Qt.AlignTop)
        body.addLayout(header)
        body.addWidget(self.pages, 1)
        footer = label("C++ captura   →   Bridge transporta   →   NumPy interpreta   →   OpenCV decodifica / Qt muestra", "muted")
        footer.setStyleSheet("font-size: 11px;")
        body.addWidget(footer)
        outer.addLayout(body, 1)
        self._capture_page()
        self._diagnostics_page()
        self._about_page()

    def _capture_page(self):
        page = QWidget()
        layout = QVBoxLayout(page)
        layout.setContentsMargins(0, 0, 0, 0)
        layout.setSpacing(14)
        source = card()
        row = QHBoxLayout(source)
        row.setContentsMargins(16, 12, 16, 12)
        row.addWidget(label("Fuente", "section"))
        self.source = QComboBox()
        self.source.setAccessibleName("Fuente de captura")
        self.source.addItem("Demo sintética · sin cámara", None)
        self.source.setMinimumWidth(220)
        self.source.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Fixed)
        row.addWidget(self.source, 1)
        self.refresh = button("Buscar", self.controller.refresh_devices, tooltip="Busca dispositivos con el motor C++. No abre la cámara.")
        self.start = button("▶  Iniciar", self._start, "primary", "Inicia la fuente seleccionada · Ctrl+Enter")
        self.stop = button("■  Detener", self._stop, "danger", "Libera la cámara y el puente · Esc")
        row.addWidget(self.refresh)
        row.addWidget(self.start)
        row.addWidget(self.stop)
        layout.addWidget(source)
        splitter = QSplitter(Qt.Horizontal)
        viewer = card()
        video_layout = QVBoxLayout(viewer)
        video_layout.setContentsMargins(12, 12, 12, 12)
        video_layout.setSpacing(10)
        toolbar = QHBoxLayout()
        toolbar.addWidget(label("VISTA EN DIRECTO", "section"), 1)
        raw = label("RAW")
        raw.setStyleSheet("color: #66e8b3; font-weight: 600;")
        toolbar.addWidget(raw)
        toolbar.addWidget(button("−", lambda: self.preview.set_zoom(self.preview.zoom / 1.25), tooltip="Alejar"))
        toolbar.addWidget(button("+", lambda: self.preview.set_zoom(self.preview.zoom * 1.25), tooltip="Ampliar"))
        toolbar.addWidget(button("Ajustar", lambda: self.preview.fit()))
        video_layout.addLayout(toolbar)
        self.preview = Preview()
        video_layout.addWidget(self.preview, 1)
        self.metadata = label("Sin frames · ajusta la vista con la rueda o Ctrl+0", "muted")
        self.metadata.setStyleSheet("font-size: 11px;")
        video_layout.addWidget(self.metadata)
        splitter.addWidget(viewer)
        info = card()
        info.setMinimumWidth(235)
        info.setMaximumWidth(310)
        panel = QVBoxLayout(info)
        panel.setContentsMargins(18, 18, 18, 18)
        panel.setSpacing(12)
        panel.addWidget(label("Estado del puente", "section"))
        self.pipeline = label("○  Motor nativo\n\n○  Memoria compartida\n\n○  Consumidor RAW", "muted")
        panel.addWidget(self.pipeline)
        self.session_info = label("Ninguna captura activa\n\nObjetivo inicial: 1280×720 / 30 FPS.\nEl formato final depende de la cámara.", "muted")
        panel.addWidget(self.session_info)
        panel.addWidget(label("Ocupación antes de consumir", "muted"))
        self.buffer_bar = QProgressBar()
        self.buffer_bar.setRange(0, 100)
        self.buffer_bar.setValue(0)
        self.buffer_bar.setTextVisible(False)
        panel.addWidget(self.buffer_bar)
        self.buffer_info = label("— · anillo de 3 slots", "muted")
        panel.addWidget(self.buffer_info)
        panel.addStretch()
        self.retry = button("Reconectar vista", self.controller.retry_view, tooltip="Reabre solo el consumidor; no reinicia la cámara.")
        panel.addWidget(self.retry)
        panel.addWidget(label("Los FPS son mediciones, no una garantía de 30 FPS.", "muted"))
        splitter.addWidget(info)
        splitter.setStretchFactor(0, 1)
        splitter.setStretchFactor(1, 0)
        splitter.setSizes([750, 270])
        layout.addWidget(splitter, 1)
        metrics = QHBoxLayout()
        metrics.setSpacing(10)
        self.capture_metric = MetricCard("FPS de captura", "Promedio del productor calculado a partir de sus timestamps.")
        self.consumer_metric = MetricCard("FPS consumidor", "Frames decodificados por segundo en la ventana reciente; no es latencia extremo a extremo.")
        self.skipped_metric = MetricCard("Frames omitidos", "Saltos de secuencia observados por este consumidor.")
        self.overwritten_metric = MetricCard("Sobrescrituras", "Frames sobrescritos por el productor antes de su consumo.")
        self.metric_cards = [self.capture_metric, self.consumer_metric, self.skipped_metric, self.overwritten_metric]
        for metric in self.metric_cards:
            metrics.addWidget(metric, 1)
        layout.addLayout(metrics)
        self.notice = label("Lista. Pulsa Iniciar para conectar el motor.", "muted")
        layout.addWidget(self.notice)
        self.pages.addWidget(page)

    def _diagnostics_page(self):
        page = QWidget()
        layout = QVBoxLayout(page)
        layout.setContentsMargins(0, 0, 0, 0)
        layout.addWidget(label("Diagnóstico de esta ejecución", "heading"))
        layout.addWidget(label("Eventos del motor y de la vista. No contiene imágenes ni video; revisa los datos antes de compartirlo.", "muted"))
        row = QHBoxLayout()
        row.addStretch()
        row.addWidget(button("Guardar diagnóstico…", self._export_diagnostics))
        layout.addLayout(row)
        self.logs = QPlainTextEdit()
        self.logs.setReadOnly(True)
        self.logs.document().setMaximumBlockCount(300)
        self.logs.setAccessibleName("Registro de diagnóstico")
        layout.addWidget(self.logs, 1)
        self.pages.addWidget(page)

    def _about_page(self):
        page = QScrollArea()
        page.setWidgetResizable(True)
        content = QWidget()
        layout = QVBoxLayout(content)
        layout.setContentsMargins(12, 12, 22, 20)
        layout.setSpacing(18)
        layout.addWidget(label("Avance 2 · una aplicación real", "heading"))
        layout.addWidget(label("Motor nativo C++ + protocolo IPC v1 + consumidor Python / NumPy / OpenCV + interfaz Qt.", "section"))
        layout.addWidget(label("Disponible en esta entrega", "section"))
        layout.addWidget(label("• Enumerar y seleccionar cámara; iniciar y detener sin terminal.\n• Fuente sintética para demostrar el puente sin webcam.\n• Vista RAW adaptable, zoom y desplazamiento.\n• Metadatos, FPS, saltos de secuencia y utilización del anillo.\n• Errores visibles, reconexión del consumidor y diagnóstico exportable.", "muted"))
        layout.addWidget(label("Fuera del avance 2", "section"))
        layout.addWidget(label("Escala de grises, bordes, detección, histórico SQLite y el dashboard de analítica corresponden a entregas posteriores. No aparecen como botones ficticios en esta aplicación.", "muted"))
        layout.addWidget(label("Atajos y privacidad", "section"))
        layout.addWidget(label("Ctrl+Enter: iniciar · Esc: detener · F11: pantalla completa · Ctrl+0: ajustar.\nRueda: zoom · Arrastrar: mover la imagen ampliada · Doble clic: ajustar.\nLos frames viven en memoria. Los logs técnicos se guardan localmente; no se suben a ningún servicio.", "muted"))
        layout.addWidget(label("Equipo", "section"))
        layout.addWidget(label("Andre Alexander Alvarado Lira\nJose Eduardo Alvarado Mejia\nJavier Gomez Correa", "muted"))
        layout.addWidget(label("Versión 0.2.1 · Interfaz para Windows 10/11 x64.\nEl núcleo conserva los backends nativos multiplataforma.\nLa licencia ORVIX es MIT; Qt/PySide se distribuyen con sus licencias correspondientes.", "muted"))
        layout.addStretch()
        page.setWidget(content)
        self.pages.addWidget(page)

    def _devices_changed(self, devices):
        selected = self.source.currentData()
        self.source.clear()
        self.source.addItem("Demo sintética · sin cámara", None)
        for item in devices:
            self.source.addItem("{} · {}".format(item.name, item.backend), item.index)
        if selected is not None:
            idx = self.source.findData(selected)
            if idx >= 0:
                self.source.setCurrentIndex(idx)
        self.refresh.setEnabled(not self.controller.active)
        self.start.setEnabled(not self.controller.active)

    def _start(self):
        if self.controller.active:
            return
        if self.controller.enumerator.state().name != "NotRunning":
            self.notice.setText("Espera a que termine la búsqueda de cámaras.")
            return
        self._started_at = time.monotonic()
        self._last_metrics = None
        for metric in self.metric_cards:
            metric.set_value("—")
        self.buffer_bar.setValue(0)
        self.preview.clear("Conectando…", "El motor prepara la fuente seleccionada.")
        self.controller.start(self.source.currentData())

    def _stop(self):
        self.controller.stop()

    def _state_changed(self, state, text):
        self.notice.setStyleSheet("")
        self._state_text = text
        self.status.setText({"idle": "○  Lista", "starting": "◌  Conectando", "running": "●  En directo",
                             "stopping": "◌  Deteniendo", "error": "!  Atención"}[state])
        self.status.setProperty("state", state)
        self.status.style().unpolish(self.status)
        self.status.style().polish(self.status)
        self.notice.setText(text)
        busy = self.controller.active or state in ("starting", "stopping")
        self.start.setEnabled(not busy)
        self.source.setEnabled(not busy)
        self.refresh.setEnabled(not busy)
        self.stop.setEnabled(busy and state != "stopping")
        self.retry.setEnabled(state == "error" and self.controller.process.state().name == "Running")
        if state == "idle":
            self.pipeline.setStyleSheet("")
            self.preview.clear("Captura detenida" if self._started_at else "Tu captura empieza aquí",
                               "Pulsa Iniciar cuando estés listo.\nLa cámara está liberada.")
            self.pipeline.setText("○  Motor detenido\n\n○  Bridge liberado\n\n○  Consumidor detenido")
            self.buffer_bar.setValue(0)
            self.buffer_info.setText("Puente liberado · sin ocupación activa")
            if self._last_metrics:
                data = self._last_metrics
                self.session_info.setText("Última captura detenida\n\n{}×{} · {}\n\nLas tarjetas muestran las últimas mediciones, no una captura activa.".format(
                    data["width"], data["height"], data["format"]))
                self.notice.setText(text + " · últimas mediciones de la captura detenida")
        elif state == "starting":
            self.pipeline.setText("◌  Preparando motor\n\n◌  Esperando Bridge\n\n○  Consumidor pendiente")
        elif state == "error":
            self.pipeline.setStyleSheet("color: #ffb6be;")
            self.preview.clear("La captura requiere atención", "Consulta Diagnóstico y reintenta.\nNo se muestra un frame antiguo como si fuera directo.")
            self.pipeline.setText("!  Revisa el diagnóstico\n\n○  Vista sin frames")

    def _connected(self, header):
        if self._started_at is None:
            self._started_at = time.monotonic()
        self.pipeline.setText("●  Motor nativo activo\n\n●  Bridge conectado\n\n●  Consumidor RAW activo")
        self.pipeline.setStyleSheet("color: #66e8b3;")
        self.session_info.setText("{}×{} · {}\nPID nativo: {}\n\nObjetivo: {:.0f} FPS\nFuente: {}".format(
            header.width, header.height, header.pixel_format, header.producer_pid,
            header.fps_numerator / max(1, header.fps_denominator), self.source.currentText()))

    def _render_latest(self):
        value = self.controller.mailbox.take()
        if value is None or self.controller.state != "running":
            return
        image, data = value
        self._last_metrics = data
        self.preview.set_image(image)
        self.capture_metric.set_value("{:.1f}".format(data["capture_fps"]))
        self.consumer_metric.set_value("{:.1f}".format(data["consumer_fps"]))
        self.skipped_metric.set_value(str(data["skipped"]))
        self.overwritten_metric.set_value(str(data["overwrites"]))
        self.buffer_bar.setValue(round(data["buffer"]))
        self.buffer_info.setText("{:.0f}% · 3 slots · último frame".format(data["buffer"]))
        self.metadata.setText("Frame #{} · {}×{} · {} · {:,} bytes · timestamp {} ×100 ns".format(
            data["sequence"], data["width"], data["height"], data["format"], data["bytes"], data["timestamp"]))
        self.notice.setText("Captura en directo · {:.0f} s · sin grabación · amplía con la rueda y arrastra para moverte".format(time.monotonic() - self._started_at))

    def _log(self, level, text):
        self.logs.appendPlainText("{}  {:7}  {}".format(datetime.now().strftime("%H:%M:%S"), level.upper(), text))
        if level in ("error", "warning"):
            self.notice.setText(text)
            self.notice.setStyleSheet("color: #ffb6be;" if level == "error" else "color: #edcd97;")

    def _export_diagnostics(self):
        destination, _filter = QFileDialog.getSaveFileName(self, "Guardar diagnóstico", "orvix-diagnostico.txt", "Texto (*.txt)")
        if destination:
            try:
                Path(destination).write_text(self.logs.toPlainText() + "\n", encoding="utf-8")
                self._log("success", "Diagnóstico guardado en el archivo elegido.")
            except OSError as error:
                QMessageBox.warning(self, "No se pudo guardar", str(error))

    def _fullscreen(self):
        self.showNormal() if self.isFullScreen() else self.showFullScreen()

    def _quiescent(self):
        if self._close_when_idle:
            QTimer.singleShot(0, self.close)

    def closeEvent(self, event: QCloseEvent):
        if self.controller.active or self.controller.enumerator.state() != QProcess.NotRunning:
            if not self._close_when_idle:
                answer = QMessageBox.question(self, "Cerrar ORVIX", "¿Detener la captura, liberar la cámara y cerrar?",
                                              QMessageBox.Yes | QMessageBox.Cancel, QMessageBox.Cancel)
                if answer != QMessageBox.Yes:
                    event.ignore()
                    return
            self._close_when_idle = True
            event.ignore()
            self.controller.shutdown()
            return
        self.controller.shutdown()
        self.settings.setValue("window/geometry", self.saveGeometry())
        event.accept()


def main(argv=None):
    parser = argparse.ArgumentParser(description="ORVIX Avance 2 desktop")
    parser.add_argument("--engine", help="Path to a native ORVIX capture executable")
    args = parser.parse_args(argv)
    app = QApplication.instance() or QApplication(sys.argv[:1])
    app.setOrganizationName("ORVIX")
    app.setApplicationName("ORVIX")
    app.setApplicationVersion("0.2.1")
    app.setStyle("Fusion")
    try:
        window = MainWindow(find_engine(args.engine))
    except (FileNotFoundError, OSError) as error:
        QMessageBox.critical(None, "No se pudo iniciar ORVIX", str(error))
        return 2
    window.show()
    return app.exec()


if __name__ == "__main__":
    raise SystemExit(main())
