# Importa os.
import os
# Importa time.
import time

# Importa pytest.
import pytest

# Ejecuta pytest.importorskip("PySide6").
pytest.importorskip("PySide6")
# Ejecuta os.environ.setdefault("QT_QPA_PLATFORM", "offscreen").
os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

# Importa QProcess, QSettings desde PySide6.QtCore.
from PySide6.QtCore import QProcess, QSettings
# Importa QApplication desde PySide6.QtWidgets.
from PySide6.QtWidgets import QApplication

# Importa History desde orvix.metrics.history.
from orvix.metrics.history import History
# Importa CaptureController, LatestFrame, find_engine, parse_devices desde orvix.ui.capture_controller.
from orvix.ui.capture_controller import CaptureController, LatestFrame, find_engine, parse_devices
# Importa MainWindow desde orvix.ui.desktop.
from orvix.ui.desktop import MainWindow


# Aplica el decorador pytest.fixture(scope="module") a la definición siguiente.
@pytest.fixture(scope="module")
# Define qt_app sin parámetros de llamada.
def qt_app():
    # Establece app a partir de QApplication.instance() or QApplication([]).
    app = QApplication.instance() or QApplication([])
    # Ejecuta app.setOrganizationName("ORVIX-Tests").
    app.setOrganizationName("ORVIX-Tests")
    # Ejecuta app.setApplicationName("ORVIX-Tests").
    app.setApplicationName("ORVIX-Tests")
    # Devuelve app.
    return app


# Define wait_until, que espera hasta una fecha límite o hasta cumplir la condición.
def wait_until(app, predicate, timeout=10):
    # Establece deadline a partir de time.monotonic() + timeout.
    deadline = time.monotonic() + timeout
    # Repite mientras not predicate() and time.monotonic() < deadline.
    while not predicate() and time.monotonic() < deadline:
        # Ejecuta app.processEvents().
        app.processEvents()
        # Ejecuta time.sleep(0.01).
        time.sleep(0.01)
    # Verifica que se cumpla predicate().
    assert predicate(), "Timed out waiting for Qt lifecycle condition"


# Comprueba native device parser and mailbox en la suite de pruebas.
def test_native_device_parser_and_mailbox():
    # Obtiene devices (dispositivos detectados) con parse_devices('[0] Cámera ñ\n Backend: Media Foundation\n Symbolic l…
    devices = parse_devices("[0] Cámera ñ\n    Backend: Media Foundation\n    Symbolic link: xyz\n\n[3] Other\n    Backend: V4L2\n")
    # Verifica que se cumpla [(d.index, d.name) for d in devices] == [(0, 'Cámera ñ'), (3, 'Other')].
    assert [(d.index, d.name) for d in devices] == [(0, "Cámera ñ"), (3, "Other")]
    # Verifica que se cumpla parse_devices('Device count: 0') == [].
    assert parse_devices("Device count: 0") == []
    # Obtiene mailbox con LatestFrame().
    mailbox = LatestFrame()
    # Ejecuta mailbox.put("old", {}).
    mailbox.put("old", {})
    # Ejecuta mailbox.put("latest", {"sequence": 2}).
    mailbox.put("latest", {"sequence": 2})
    # Verifica que se cumpla mailbox.take() == ('latest', {'sequence': 2}).
    assert mailbox.take() == ("latest", {"sequence": 2})
    # Verifica que se cumpla mailbox.take() is None.
    assert mailbox.take() is None


# Comprueba missing explicit engine is not silently replaced en la suite de pruebas.
def test_missing_explicit_engine_is_not_silently_replaced(tmp_path):
    # Acota el uso de pytest.raises(FileNotFoundError) a este bloque.
    with pytest.raises(FileNotFoundError):
        # Ejecuta find_engine(str(tmp_path / "missing.exe")).
        find_engine(str(tmp_path / "missing.exe"))


# Comprueba shutdown finishes when device enumerator cannot start en la suite de pruebas.
def test_shutdown_finishes_when_device_enumerator_cannot_start(qt_app, tmp_path):
    # Obtiene controller con CaptureController(tmp_path / 'missing.exe', work_dir=tmp_path).
    controller = CaptureController(tmp_path / "missing.exe", work_dir=tmp_path)
    # Inicializa finished con [].
    finished = []
    # Conecta el evento con su manejador mediante controller.quiescent.connect con lambda: finished.append(True).
    controller.quiescent.connect(lambda: finished.append(True))
    # Ejecuta controller.refresh_devices().
    controller.refresh_devices()
    # Ejecuta controller.shutdown().
    controller.shutdown()
    # Espera hasta una fecha límite o hasta cumplir la condición mediante wait_until con qt_app, lambda: bool(finished).
    wait_until(qt_app, lambda: bool(finished))
    # Verifica que se cumpla controller.enumerator.state() == QProcess.NotRunning.
    assert controller.enumerator.state() == QProcess.NotRunning
    # Verifica que se cumpla not controller.active.
    assert not controller.active


# Comprueba widget layout and zoom en la suite de pruebas.
def test_widget_layout_and_zoom(qt_app, tmp_path):
    # Obtiene window con MainWindow(find_engine(), tmp_path).
    window = MainWindow(find_engine(), tmp_path)
    # Ejecuta el bloque con manejo de excepciones.
    try:
        # Ajusta el tamaño del contenedor mediante window.resize con 1024, 768.
        window.resize(1024, 768)
        # Ejecuta window.show().
        window.show()
        # Ejecuta qt_app.processEvents().
        qt_app.processEvents()
        # Verifica que se cumpla window.width() == 1024 and window.height() == 768.
        assert window.width() == 1024 and window.height() == 768
        # Verifica que se cumpla window.preview.width() >= 300.
        assert window.preview.width() >= 300
        # Verifica que se cumpla window.preview.height() >= 210.
        assert window.preview.height() >= 210
        # Verifica que se cumpla window.start.isEnabled().
        assert window.start.isEnabled()
        # Verifica que se cumpla not window.stop.isEnabled().
        assert not window.stop.isEnabled()
        # Verifica que se cumpla window.preview.image.isNull().
        assert window.preview.image.isNull()
        # Verifica que se cumpla window.pages.count() == 5.
        assert window.pages.count() == 5
        # Ejecuta window.preview.set_zoom(20).
        window.preview.set_zoom(20)
        # Verifica que se cumpla window.preview.zoom == 5.
        assert window.preview.zoom == 5
        # Ejecuta window.preview.fit().
        window.preview.fit()
        # Verifica que se cumpla window.preview.zoom == 1.
        assert window.preview.zoom == 1
        # All controls remain within the client area at the supported minimum.
        # Recorre [window.source, window.start, window.stop, window.overwritten_metric] y toma cada elemento como control.
        for control in [window.source, window.start, window.stop, window.overwritten_metric]:
            # Obtiene point con control.mapTo(window, control.rect().bottomRight()).
            point = control.mapTo(window, control.rect().bottomRight())
            # Verifica que se cumpla 0 <= point.x() < window.width().
            assert 0 <= point.x() < window.width()
            # Verifica que se cumpla 0 <= point.y() < window.height().
            assert 0 <= point.y() < window.height()
    # Ejecuta la limpieza al salir del bloque, incluso si hubo un error.
    finally:
        # Ejecuta window.controller.shutdown().
        window.controller.shutdown()
        # Espera hasta una fecha límite o hasta cumplir la condición mediante wait_until con qt_app, lambda: window.controller.enumerator.state() == QPro…
        wait_until(qt_app, lambda: window.controller.enumerator.state() == QProcess.NotRunning)
        # Cierra el recurso mediante window.close.
        window.close()


# Aplica el decorador pytest.mark.integration a la definición siguiente.
@pytest.mark.integration
# Comprueba invalid camera and cancel during startup en la suite de pruebas.
def test_invalid_camera_and_cancel_during_startup(qt_app, tmp_path):
    # Obtiene window con MainWindow(find_engine(), tmp_path).
    window = MainWindow(find_engine(), tmp_path)
    # Ejecuta el bloque con manejo de excepciones.
    try:
        # Ejecuta window.controller.start(999999).
        window.controller.start(999999)
        # Espera hasta una fecha límite o hasta cumplir la condición mediante wait_until con qt_app, lambda: not window.controller.active.
        wait_until(qt_app, lambda: not window.controller.active)
        # Verifica que se cumpla window.controller.state == 'error'.
        assert window.controller.state == "error"
        # Verifica que se cumpla 'ORV-CAP' in window.logs.toPlainText().
        assert "ORV-CAP" in window.logs.toPlainText()
        # Ejecuta window.controller.start().
        window.controller.start()
        # Ejecuta window.controller.stop().
        window.controller.stop()
        # Espera hasta una fecha límite o hasta cumplir la condición mediante wait_until con qt_app, lambda: not window.controller.active.
        wait_until(qt_app, lambda: not window.controller.active)
        # Verifica que se cumpla window.controller.state == 'idle'.
        assert window.controller.state == "idle"
    # Ejecuta la limpieza al salir del bloque, incluso si hubo un error.
    finally:
        # Ejecuta window.controller.shutdown().
        window.controller.shutdown()
        # Espera hasta una fecha límite o hasta cumplir la condición mediante wait_until con qt_app, lambda: not window.controller.active.
        wait_until(qt_app, lambda: not window.controller.active)
        # Espera hasta una fecha límite o hasta cumplir la condición mediante wait_until con qt_app, lambda: window.controller.enumerator.state() == QPro…
        wait_until(qt_app, lambda: window.controller.enumerator.state() == QProcess.NotRunning)
        # Cierra el recurso mediante window.close.
        window.close()


# Aplica el decorador pytest.mark.integration a la definición siguiente.
@pytest.mark.integration
# Comprueba consumer failure does not kill motor and view reconnects en la suite de pruebas.
def test_consumer_failure_does_not_kill_motor_and_view_reconnects(qt_app, tmp_path):
    # Obtiene window con MainWindow(find_engine(), tmp_path).
    window = MainWindow(find_engine(), tmp_path)
    # Ejecuta el bloque con manejo de excepciones.
    try:
        # Ejecuta window._start().
        window._start()
        # Espera hasta una fecha límite o hasta cumplir la condición mediante wait_until con qt_app, lambda: window._last_metrics is not None.
        wait_until(qt_app, lambda: window._last_metrics is not None)
        # Obtiene pid con window.controller.process.processId().
        pid = window.controller.process.processId()
        # Ejecuta window.controller.worker.requestInterruption().
        window.controller.worker.requestInterruption()
        # Espera hasta una fecha límite o hasta cumplir la condición mediante wait_until con qt_app, lambda: window.controller.worker is None.
        wait_until(qt_app, lambda: window.controller.worker is None)
        # Ejecuta window.controller._reader_failed("Simulated consumer failure").
        window.controller._reader_failed("Simulated consumer failure")
        # Verifica que se cumpla window.controller.process.state() == QProcess.Running.
        assert window.controller.process.state() == QProcess.Running
        # Ejecuta window.controller.retry_view().
        window.controller.retry_view()
        # Espera hasta una fecha límite o hasta cumplir la condición mediante wait_until con qt_app, lambda: window.controller.state == 'running'.
        wait_until(qt_app, lambda: window.controller.state == "running")
        # Verifica que se cumpla window.controller.process.processId() == pid.
        assert window.controller.process.processId() == pid
        # Ejecuta window.controller.stop().
        window.controller.stop()
        # Espera hasta una fecha límite o hasta cumplir la condición mediante wait_until con qt_app, lambda: not window.controller.active.
        wait_until(qt_app, lambda: not window.controller.active)
        # Verifica que se cumpla window.controller.process.exitCode() == 0.
        assert window.controller.process.exitCode() == 0
    # Ejecuta la limpieza al salir del bloque, incluso si hubo un error.
    finally:
        # Ejecuta window.controller.shutdown().
        window.controller.shutdown()
        # Espera hasta una fecha límite o hasta cumplir la condición mediante wait_until con qt_app, lambda: not window.controller.active.
        wait_until(qt_app, lambda: not window.controller.active)
        # Espera hasta una fecha límite o hasta cumplir la condición mediante wait_until con qt_app, lambda: window.controller.enumerator.state() == QPro…
        wait_until(qt_app, lambda: window.controller.enumerator.state() == QProcess.NotRunning)
        # Cierra el recurso mediante window.close.
        window.close()


# Aplica el decorador pytest.mark.integration a la definición siguiente.
@pytest.mark.integration
# Comprueba desktop real bridge start stop restart and crash en la suite de pruebas.
def test_desktop_real_bridge_start_stop_restart_and_crash(qt_app, tmp_path):
    # Obtiene window con MainWindow(find_engine(), tmp_path).
    window = MainWindow(find_engine(), tmp_path)
    # Ejecuta el bloque con manejo de excepciones.
    try:
        # Ejecuta window.show().
        window.show()
        # Recorre range(2) y toma cada elemento como _.
        for _ in range(2):
            # Ejecuta window._start().
            window._start()
            # Espera hasta una fecha límite o hasta cumplir la condición mediante wait_until con qt_app, lambda: window._last_metrics is not None.
            wait_until(qt_app, lambda: window._last_metrics is not None)
            # Verifica que se cumpla window.controller.state == 'running'.
            assert window.controller.state == "running"
            # Verifica que se cumpla not window.start.isEnabled().
            assert not window.start.isEnabled()
            # Verifica que se cumpla window.stop.isEnabled().
            assert window.stop.isEnabled()
            # Verifica que se cumpla not window.preview.image.isNull().
            assert not window.preview.image.isNull()
            # Verifica que se cumpla window._last_metrics['format'] == 'NV12'.
            assert window._last_metrics["format"] == "NV12"
            # Ejecuta window._stop().
            window._stop()
            # Espera hasta una fecha límite o hasta cumplir la condición mediante wait_until con qt_app, lambda: not window.controller.active.
            wait_until(qt_app, lambda: not window.controller.active)
            # Verifica que se cumpla window.controller.process.exitCode() == 0.
            assert window.controller.process.exitCode() == 0, window.logs.toPlainText()
            # Verifica que se cumpla window.controller.state == 'idle'.
            assert window.controller.state == "idle"
            # Verifica que se cumpla window.start.isEnabled().
            assert window.start.isEnabled()
            # Verifica que se cumpla window.preview.image.isNull().
            assert window.preview.image.isNull()
        # Ejecuta window._start().
        window._start()
        # Espera hasta una fecha límite o hasta cumplir la condición mediante wait_until con qt_app, lambda: window._last_metrics is not None.
        wait_until(qt_app, lambda: window._last_metrics is not None)
        # Ejecuta window.controller.process.kill() # Only this test's owned native process.
        window.controller.process.kill()  # Only this test's owned native process.
        # Espera hasta una fecha límite o hasta cumplir la condición mediante wait_until con qt_app, lambda: not window.controller.active.
        wait_until(qt_app, lambda: not window.controller.active)
        # Verifica que se cumpla window.controller.state == 'error'.
        assert window.controller.state == "error"
        # Verifica que se cumpla window.start.isEnabled().
        assert window.start.isEnabled()
        # Verifica que se cumpla window.preview.image.isNull().
        assert window.preview.image.isNull()
    # Ejecuta la limpieza al salir del bloque, incluso si hubo un error.
    finally:
        # Ejecuta window.controller.shutdown().
        window.controller.shutdown()
        # Espera hasta una fecha límite o hasta cumplir la condición mediante wait_until con qt_app, lambda: not window.controller.active.
        wait_until(qt_app, lambda: not window.controller.active)
        # Espera hasta una fecha límite o hasta cumplir la condición mediante wait_until con qt_app, lambda: window.controller.enumerator.state() == QPro…
        wait_until(qt_app, lambda: window.controller.enumerator.state() == QProcess.NotRunning)
        # Cierra el recurso mediante window.close.
        window.close()

# Aplica el decorador pytest.mark.integration a la definición siguiente.
@pytest.mark.integration
# Comprueba live modes and history survive reopen en la suite de pruebas.
def test_live_modes_and_history_survive_reopen(qt_app, tmp_path, monkeypatch):
    # Ejecuta monkeypatch.setattr("orvix.ui.desktop.QSettings", lambda: QSettings(str(tmp_path / "app…
    monkeypatch.setattr("orvix.ui.desktop.QSettings", lambda: QSettings(str(tmp_path / "app.ini"), QSettings.IniFormat))
    # Obtiene window con MainWindow(find_engine(), tmp_path).
    window = MainWindow(find_engine(), tmp_path)
    # Ejecuta el bloque con manejo de excepciones.
    try:
        # Ejecuta window.profile.setCurrentIndex(window.profile.findData("480p30")).
        window.profile.setCurrentIndex(window.profile.findData("480p30"))
        # Establece el valor del control o preferencia mediante window.slots.setValue con 4.
        window.slots.setValue(4)
        # Ejecuta window._start().
        window._start()
        # Espera hasta una fecha límite o hasta cumplir la condición mediante wait_until con qt_app, lambda: window._last_metrics is not None.
        wait_until(qt_app, lambda: window._last_metrics is not None)
        # Establece before a partir de window._last_metrics['sequence'].
        before = window._last_metrics["sequence"]
        # Obtiene pid con window.controller.process.processId().
        pid = window.controller.process.processId()
        # Verifica que se cumpla (window._last_metrics['width'], window._last_metrics['height']) == (640, 480).
        assert (window._last_metrics["width"], window._last_metrics["height"]) == (640, 480)
        # Verifica que se cumpla window._last_metrics['slots'] == 4.
        assert window._last_metrics["slots"] == 4
        # Recorre ('gray', 'edges', 'motion') y toma cada elemento como mode.
        for mode in ("gray", "edges", "motion"):
            # Ejecuta window.mode.setCurrentIndex(window.mode.findData(mode)).
            window.mode.setCurrentIndex(window.mode.findData(mode))
            # Espera hasta una fecha límite o hasta cumplir la condición mediante wait_until con qt_app, lambda: window._last_metrics['mode'] == mode.
            wait_until(qt_app, lambda: window._last_metrics["mode"] == mode)
            # Verifica que se cumpla window._last_metrics['sequence'] > before.
            assert window._last_metrics["sequence"] > before
            # Verifica que se cumpla window.controller.process.processId() == pid.
            assert window.controller.process.processId() == pid
            # Verifica que se cumpla window._last_metrics['latency_ms'] > 0.
            assert window._last_metrics["latency_ms"] > 0
        # Ejecuta window._stop().
        window._stop()
        # Espera hasta una fecha límite o hasta cumplir la condición mediante wait_until con qt_app, lambda: not window.controller.active.
        wait_until(qt_app, lambda: not window.controller.active)
        # Ejecuta window._navigate(1).
        window._navigate(1)
        # Verifica que se cumpla window.history_page.sessions.count() == 1.
        assert window.history_page.sessions.count() == 1
        # Verifica que se cumpla 'Finalizada' in window.history_page.details.text().
        assert "Finalizada" in window.history_page.details.text()
    # Ejecuta la limpieza al salir del bloque, incluso si hubo un error.
    finally:
        # Ejecuta window.controller.shutdown().
        window.controller.shutdown()
        # Espera hasta una fecha límite o hasta cumplir la condición mediante wait_until con qt_app, lambda: not window.controller.active.
        wait_until(qt_app, lambda: not window.controller.active)
        # Espera hasta una fecha límite o hasta cumplir la condición mediante wait_until con qt_app, lambda: window.controller.enumerator.state() == QPro…
        wait_until(qt_app, lambda: window.controller.enumerator.state() == QProcess.NotRunning)
        # Cierra el recurso mediante window.close.
        window.close()

    # Obtiene history (historial SQLite) con History(tmp_path / 'telemetry.sqlite3').
    history = History(tmp_path / "telemetry.sqlite3")
    # Ejecuta el bloque con manejo de excepciones.
    try:
        # Obtiene (session,) con history.sessions().
        session, = history.sessions()
        # Verifica que se cumpla (session['state'], session['profile'], session['width']) == ('stopped', '480p30', 640).
        assert (session["state"], session["profile"], session["width"]) == ("stopped", "480p30", 640)
        # Verifica que se cumpla history.samples(session['session_id'])[-1]['mode'] == 'motion'.
        assert history.samples(session["session_id"])[-1]["mode"] == "motion"
        # Verifica que se cumpla any(('Movimiento' in event['message'] for event in history.events(session['session_id']…
        assert any("Movimiento" in event["message"] for event in history.events(session["session_id"]))
    # Ejecuta la limpieza al salir del bloque, incluso si hubo un error.
    finally:
        # Cierra el recurso mediante history.close.
        history.close()
