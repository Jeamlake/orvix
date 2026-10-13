import os
import time

import pytest

pytest.importorskip("PySide6")
os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from PySide6.QtCore import QProcess
from PySide6.QtWidgets import QApplication

from orvix.ui.capture_controller import CaptureController, LatestFrame, find_engine, parse_devices
from orvix.ui.desktop import MainWindow


@pytest.fixture(scope="module")
def qt_app():
    app = QApplication.instance() or QApplication([])
    app.setOrganizationName("ORVIX-Tests")
    app.setApplicationName("ORVIX-Tests")
    return app


def wait_until(app, predicate, timeout=10):
    deadline = time.monotonic() + timeout
    while not predicate() and time.monotonic() < deadline:
        app.processEvents()
        time.sleep(0.01)
    assert predicate(), "Timed out waiting for Qt lifecycle condition"


def test_native_device_parser_and_mailbox():
    devices = parse_devices("[0] Cámera ñ\n    Backend: Media Foundation\n    Symbolic link: xyz\n\n[3] Other\n    Backend: V4L2\n")
    assert [(d.index, d.name) for d in devices] == [(0, "Cámera ñ"), (3, "Other")]
    assert parse_devices("Device count: 0") == []
    mailbox = LatestFrame()
    mailbox.put("old", {})
    mailbox.put("latest", {"sequence": 2})
    assert mailbox.take() == ("latest", {"sequence": 2})
    assert mailbox.take() is None


def test_missing_explicit_engine_is_not_silently_replaced(tmp_path):
    with pytest.raises(FileNotFoundError):
        find_engine(str(tmp_path / "missing.exe"))


def test_shutdown_finishes_when_device_enumerator_cannot_start(qt_app, tmp_path):
    controller = CaptureController(tmp_path / "missing.exe", work_dir=tmp_path)
    finished = []
    controller.quiescent.connect(lambda: finished.append(True))
    controller.refresh_devices()
    controller.shutdown()
    wait_until(qt_app, lambda: bool(finished))
    assert controller.enumerator.state() == QProcess.NotRunning
    assert not controller.active


def test_widget_layout_and_zoom(qt_app, tmp_path):
    window = MainWindow(find_engine(), tmp_path)
    try:
        window.resize(1024, 768)
        window.show()
        qt_app.processEvents()
        assert window.width() == 1024 and window.height() == 768
        assert window.preview.width() >= 300
        assert window.preview.height() >= 210
        assert window.start.isEnabled()
        assert not window.stop.isEnabled()
        assert window.preview.image.isNull()
        assert window.pages.count() == 3
        window.preview.set_zoom(20)
        assert window.preview.zoom == 5
        window.preview.fit()
        assert window.preview.zoom == 1
        # All controls remain within the client area at the supported minimum.
        for control in [window.source, window.start, window.stop, window.overwritten_metric]:
            point = control.mapTo(window, control.rect().bottomRight())
            assert 0 <= point.x() < window.width()
            assert 0 <= point.y() < window.height()
    finally:
        window.controller.shutdown()
        wait_until(qt_app, lambda: window.controller.enumerator.state() == QProcess.NotRunning)
        window.close()


@pytest.mark.integration
def test_invalid_camera_and_cancel_during_startup(qt_app, tmp_path):
    window = MainWindow(find_engine(), tmp_path)
    try:
        window.controller.start(999999)
        wait_until(qt_app, lambda: not window.controller.active)
        assert window.controller.state == "error"
        assert "ORV-CAP" in window.logs.toPlainText()
        window.controller.start()
        window.controller.stop()
        wait_until(qt_app, lambda: not window.controller.active)
        assert window.controller.state == "idle"
    finally:
        window.controller.shutdown()
        wait_until(qt_app, lambda: not window.controller.active)
        wait_until(qt_app, lambda: window.controller.enumerator.state() == QProcess.NotRunning)
        window.close()


@pytest.mark.integration
def test_consumer_failure_does_not_kill_motor_and_view_reconnects(qt_app, tmp_path):
    window = MainWindow(find_engine(), tmp_path)
    try:
        window._start()
        wait_until(qt_app, lambda: window._last_metrics is not None)
        pid = window.controller.process.processId()
        window.controller.worker.requestInterruption()
        wait_until(qt_app, lambda: window.controller.worker is None)
        window.controller._reader_failed("Simulated consumer failure")
        assert window.controller.process.state() == QProcess.Running
        window.controller.retry_view()
        wait_until(qt_app, lambda: window.controller.state == "running")
        assert window.controller.process.processId() == pid
        window.controller.stop()
        wait_until(qt_app, lambda: not window.controller.active)
        assert window.controller.process.exitCode() == 0
    finally:
        window.controller.shutdown()
        wait_until(qt_app, lambda: not window.controller.active)
        wait_until(qt_app, lambda: window.controller.enumerator.state() == QProcess.NotRunning)
        window.close()


@pytest.mark.integration
def test_desktop_real_bridge_start_stop_restart_and_crash(qt_app, tmp_path):
    window = MainWindow(find_engine(), tmp_path)
    try:
        window.show()
        for _ in range(2):
            window._start()
            wait_until(qt_app, lambda: window._last_metrics is not None)
            assert window.controller.state == "running"
            assert not window.start.isEnabled()
            assert window.stop.isEnabled()
            assert not window.preview.image.isNull()
            assert window._last_metrics["format"] == "NV12"
            window._stop()
            wait_until(qt_app, lambda: not window.controller.active)
            assert window.controller.process.exitCode() == 0, window.logs.toPlainText()
            assert window.controller.state == "idle"
            assert window.start.isEnabled()
            assert window.preview.image.isNull()
        window._start()
        wait_until(qt_app, lambda: window._last_metrics is not None)
        window.controller.process.kill()  # Only this test's owned native process.
        wait_until(qt_app, lambda: not window.controller.active)
        assert window.controller.state == "error"
        assert window.start.isEnabled()
        assert window.preview.image.isNull()
    finally:
        window.controller.shutdown()
        wait_until(qt_app, lambda: not window.controller.active)
        wait_until(qt_app, lambda: window.controller.enumerator.state() == QProcess.NotRunning)
        window.close()
