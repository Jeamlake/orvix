"""Resizable, keyboard-accessible desktop widgets; no camera ownership here."""

from __future__ import annotations

from PySide6.QtCore import QPointF, QRectF, Qt, Signal
from PySide6.QtGui import QColor, QFont, QImage, QPainter, QPen
from PySide6.QtWidgets import QFrame, QLabel, QVBoxLayout, QWidget


STYLE = """
QMainWindow, QWidget#shell { background: #101018; }
QWidget { color: #efecff; font-family: 'Segoe UI'; font-size: 13px; }
QFrame#sidebar { background: #0c0c14; border-right: 1px solid #252436; }
QFrame#card { background: #171722; border: 1px solid #2b293f; border-radius: 12px; }
QLabel { background: transparent; border: none; }
QLabel[role='muted'] { color: #a3a0b8; }
QLabel[role='heading'] { font-size: 22px; font-weight: 600; }
QLabel[role='section'] { font-size: 14px; font-weight: 600; }
QLabel[role='value'] { font-size: 27px; font-weight: 600; color: #66e8b3; }
QLabel#status { background: #222032; color: #b8a1ff; border: 1px solid #3b3455;
    padding: 7px 13px; border-radius: 15px; font-weight: 600; }
QLabel#status[state='running'] { background: #13352c; color: #66e8b3; border-color: #225542; }
QLabel#status[state='error'] { background: #3a222a; color: #ffb2b9; border-color: #69404c; }
QPushButton { background: #252335; border: 1px solid #3c3754; border-radius: 8px;
    padding: 9px 15px; min-height: 18px; font-weight: 600; }
QPushButton:hover { background: #363047; border-color: #a183f0; }
QPushButton:pressed { background: #48385c; }
QPushButton:focus, QComboBox:focus { border: 2px solid #b49aff; }
QPushButton:disabled { color: #777486; background: #1c1b26; border-color: #292636; }
QPushButton#primary { background: #8d69df; color: #ffffff; border-color: #ac8cee; }
QPushButton#primary:hover { background: #a17dea; }
QPushButton#primary:disabled { background: #312842; border-color: #44344f; color: #8e809e; }
QPushButton#danger { background: #392630; border-color: #66414f; color: #ffbcc7; }
QPushButton#danger:disabled { background: #1c1b26; border-color: #292636; color: #777486; }
QPushButton#nav { text-align: left; background: transparent; border: 1px solid transparent; padding: 12px; }
QPushButton#nav:checked { background: #28203e; color: #c6aaff; border-color: #46315f; }
QPushButton#nav:hover { background: #211e2d; }
QComboBox { background: #20202f; border: 1px solid #3d3851; border-radius: 8px;
    padding: 8px 12px; min-height: 20px; }
QComboBox:disabled { color: #858196; }
QComboBox::drop-down { width: 26px; border: none; }
QComboBox QAbstractItemView { background: #242033; selection-background-color: #62428d; padding: 6px; }
QPlainTextEdit, QListWidget { background: #11111c; border: 1px solid #2e2a41;
    border-radius: 10px; padding: 10px; selection-background-color: #5c427d; }
QScrollArea { border: none; background: transparent; }
QScrollBar:vertical { background: #171722; width: 9px; margin: 0; }
QScrollBar::handle:vertical { background: #4b4163; border-radius: 4px; min-height: 28px; }
QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }
QProgressBar { background: #282337; border: none; border-radius: 4px; min-height: 7px; max-height: 7px; }
QProgressBar::chunk { background: #9e7ae9; border-radius: 4px; }
QSplitter::handle { background: #101018; width: 12px; }
QToolTip { background: #302740; color: #f8f4ff; border: 1px solid #796195; padding: 7px; }
QMessageBox { background: #171722; }
"""


def label(text, role=None):
    widget = QLabel(text)
    if role:
        widget.setProperty("role", role)
    widget.setWordWrap(True)
    return widget


def card():
    widget = QFrame()
    widget.setObjectName("card")
    return widget


class MetricCard(QFrame):
    def __init__(self, title, help_text, parent=None):
        super().__init__(parent)
        self.setObjectName("card")
        layout = QVBoxLayout(self)
        layout.setContentsMargins(16, 12, 16, 12)
        layout.setSpacing(3)
        layout.addWidget(label(title, "muted"))
        self.value = label("—", "value")
        self.value.setAccessibleName(title)
        layout.addWidget(self.value)
        self.setToolTip(help_text)
        self.setMinimumWidth(110)

    def set_value(self, value):
        self.value.setText(value)


class Preview(QWidget):
    """Aspect-fit video; wheel zoom and bounded drag at zoom > fit."""
    zoom_changed = Signal(float)

    def __init__(self, parent=None):
        super().__init__(parent)
        self.setMinimumSize(300, 210)
        self.setFocusPolicy(Qt.StrongFocus)
        self.setAccessibleName("Vista RAW. Rueda para ampliar; arrastra para mover; doble clic para ajustar.")
        self.image = QImage()
        self.zoom = 1.0
        self.offset = QPointF()
        self._drag = None
        self.message = "Tu captura empieza aquí"
        self.detail = "Elige una cámara o prueba la demo.\nLa cámara solo se activa al pulsar Iniciar."

    def set_image(self, bgr):
        height, width = bgr.shape[:2]
        self.image = QImage(bgr.data, width, height, bgr.strides[0], QImage.Format_BGR888).copy()
        self.update()

    def clear(self, message, detail):
        self.image = QImage()
        self.message, self.detail = message, detail
        self.fit()

    def fit(self):
        self.zoom = 1.0
        self.offset = QPointF()
        self.zoom_changed.emit(self.zoom)
        self.update()

    def set_zoom(self, factor):
        self.zoom = max(1.0, min(5.0, factor))
        self._clamp()
        self.zoom_changed.emit(self.zoom)
        self.update()

    def _size(self):
        if self.image.isNull():
            return (0.0, 0.0)
        scale = min(self.width() / self.image.width(), self.height() / self.image.height()) * self.zoom
        return self.image.width() * scale, self.image.height() * scale

    def _clamp(self):
        width, height = self._size()
        dx, dy = max(0.0, (width - self.width()) / 2), max(0.0, (height - self.height()) / 2)
        self.offset = QPointF(max(-dx, min(dx, self.offset.x())), max(-dy, min(dy, self.offset.y())))

    def paintEvent(self, _event):
        painter = QPainter(self)
        painter.setRenderHint(QPainter.Antialiasing)
        painter.fillRect(self.rect(), QColor("#090911"))
        if not self.image.isNull():
            width, height = self._size()
            destination = QRectF((self.width() - width) / 2 + self.offset.x(),
                                 (self.height() - height) / 2 + self.offset.y(), width, height)
            painter.setRenderHint(QPainter.SmoothPixmapTransform)
            painter.drawImage(destination, self.image)
            return
        cx, cy = self.width() / 2, self.height() / 2 - 55
        painter.setPen(QPen(QColor("#44375f"), 2))
        painter.drawEllipse(QPointF(cx, cy), 43, 43)
        painter.setPen(QPen(QColor("#a985f2"), 3))
        painter.drawEllipse(QPointF(cx, cy), 24, 24)
        painter.setPen(QPen(QColor("#66e8b3"), 3))
        painter.drawArc(QRectF(cx - 34, cy - 34, 68, 68), 35 * 16, 100 * 16)
        painter.setPen(QColor("#ece6ff"))
        painter.setFont(QFont("Segoe UI", 16, QFont.DemiBold))
        painter.drawText(QRectF(18, cy + 58, self.width() - 36, 50), Qt.AlignHCenter | Qt.TextWordWrap, self.message)
        painter.setFont(QFont("Segoe UI", 10))
        painter.setPen(QColor("#aaa2bd"))
        painter.drawText(QRectF(18, cy + 111, self.width() - 36, 65), Qt.AlignHCenter | Qt.TextWordWrap, self.detail)

    def resizeEvent(self, event):
        self._clamp()
        super().resizeEvent(event)

    def wheelEvent(self, event):
        if not self.image.isNull():
            self.set_zoom(self.zoom * (1.15 if event.angleDelta().y() > 0 else 1 / 1.15))
        event.accept()

    def mousePressEvent(self, event):
        if event.button() == Qt.LeftButton and self.zoom > 1:
            self._drag = event.position()
            self.setCursor(Qt.ClosedHandCursor)

    def mouseMoveEvent(self, event):
        if self._drag is not None:
            self.offset += event.position() - self._drag
            self._drag = event.position()
            self._clamp()
            self.update()

    def mouseReleaseEvent(self, event):
        self._drag = None
        self.unsetCursor()

    def mouseDoubleClickEvent(self, event):
        self.fit()
