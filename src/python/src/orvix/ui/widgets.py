"""Resizable, keyboard-accessible desktop widgets; no camera ownership here."""

# Importa annotations desde __future__.
from __future__ import annotations

# Importa QPointF, QRectF, Qt, Signal desde PySide6.QtCore.
from PySide6.QtCore import QPointF, QRectF, Qt, Signal
# Importa QColor, QFont, QImage, QPainter, QPen desde PySide6.QtGui.
from PySide6.QtGui import QColor, QFont, QImage, QPainter, QPen
# Importa QFrame, QLabel, QVBoxLayout, QWidget desde PySide6.QtWidgets.
from PySide6.QtWidgets import QFrame, QLabel, QVBoxLayout, QWidget


# Establece STYLE a partir de "\nQMainWindow, QWidget#shell { background: #101018; }\nQWidget { color: #efecff;…
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


# Define label con los parámetros text, role.
def label(text, role=None):
    # Obtiene widget con QLabel(text).
    widget = QLabel(text)
    # Entra en esta rama si role.
    if role:
        # Ejecuta widget.setProperty("role", role).
        widget.setProperty("role", role)
    # Ejecuta widget.setWordWrap(True).
    widget.setWordWrap(True)
    # Devuelve widget.
    return widget


# Define card sin parámetros de llamada.
def card():
    # Obtiene widget con QFrame().
    widget = QFrame()
    # Ejecuta widget.setObjectName("card").
    widget.setObjectName("card")
    # Devuelve widget.
    return widget


# Declara el tipo MetricCard que hereda de QFrame.
class MetricCard(QFrame):
    # Inicializa la instancia con title, help_text, parent.
    def __init__(self, title, help_text, parent=None):
        # Ejecuta super().__init__(parent).
        super().__init__(parent)
        # Ejecuta self.setObjectName("card").
        self.setObjectName("card")
        # Obtiene layout con QVBoxLayout(self).
        layout = QVBoxLayout(self)
        # Ejecuta layout.setContentsMargins(16, 12, 16, 12).
        layout.setContentsMargins(16, 12, 16, 12)
        # Ejecuta layout.setSpacing(3).
        layout.setSpacing(3)
        # Agrega el control al diseño de la ventana mediante layout.addWidget con label(title, 'muted').
        layout.addWidget(label(title, "muted"))
        # Obtiene self.value con label('—', 'value').
        self.value = label("—", "value")
        # Ejecuta self.value.setAccessibleName(title).
        self.value.setAccessibleName(title)
        # Agrega el control al diseño de la ventana mediante layout.addWidget con self.value.
        layout.addWidget(self.value)
        # Ejecuta self.setToolTip(help_text).
        self.setToolTip(help_text)
        # Ejecuta self.setMinimumWidth(110).
        self.setMinimumWidth(110)

    # Define set_value con el parámetro value.
    def set_value(self, value):
        # Actualiza el texto visible del control mediante self.value.setText con value.
        self.value.setText(value)


# Declara el tipo Preview que hereda de QWidget.
class Preview(QWidget):
    """Aspect-fit video; wheel zoom and bounded drag at zoom > fit."""
    # Obtiene zoom_changed con Signal(float).
    zoom_changed = Signal(float)

    # Inicializa la instancia con parent.
    def __init__(self, parent=None):
        # Ejecuta super().__init__(parent).
        super().__init__(parent)
        # Ejecuta self.setMinimumSize(300, 210).
        self.setMinimumSize(300, 210)
        # Ejecuta self.setFocusPolicy(Qt.StrongFocus).
        self.setFocusPolicy(Qt.StrongFocus)
        # Ejecuta self.setAccessibleName("Vista en directo. Rueda para ampliar; arrastra para mover; dobl…
        self.setAccessibleName("Vista en directo. Rueda para ampliar; arrastra para mover; doble clic para ajustar.")
        # Obtiene self.image (imagen) con QImage().
        self.image = QImage()
        # Establece self.zoom a partir de 1.0.
        self.zoom = 1.0
        # Obtiene self.offset con QPointF().
        self.offset = QPointF()
        # Establece self._drag a partir de None.
        self._drag = None
        # Establece self.message a partir de 'Tu captura empieza aquí'.
        self.message = "Tu captura empieza aquí"
        # Establece self.detail a partir de 'Elige una cámara o prueba la demo.\nLa cámara solo se activa al pulsar Iniciar.'.
        self.detail = "Elige una cámara o prueba la demo.\nLa cámara solo se activa al pulsar Iniciar."

    # Define set_image con el parámetro bgr.
    def set_image(self, bgr):
        # Establece (height, width) a partir de bgr.shape[:2].
        height, width = bgr.shape[:2]
        # Obtiene self.image (imagen) con QImage(bgr.data, width, height, bgr.strides[0], QImage.Format_BGR888…
        self.image = QImage(bgr.data, width, height, bgr.strides[0], QImage.Format_BGR888).copy()
        # Ejecuta self.update().
        self.update()

    # Define clear, que vacía el contenedor.
    def clear(self, message, detail):
        # Obtiene self.image (imagen) con QImage().
        self.image = QImage()
        # Inicializa (self.message, self.detail) con (message, detail).
        self.message, self.detail = message, detail
        # Ejecuta self.fit().
        self.fit()

    # Define fit sin parámetros de llamada.
    def fit(self):
        # Establece self.zoom a partir de 1.0.
        self.zoom = 1.0
        # Obtiene self.offset con QPointF().
        self.offset = QPointF()
        # Publica la señal para sus receptores mediante self.zoom_changed.emit con self.zoom.
        self.zoom_changed.emit(self.zoom)
        # Ejecuta self.update().
        self.update()

    # Define set_zoom con el parámetro factor.
    def set_zoom(self, factor):
        # Obtiene self.zoom con max(1.0, min(5.0, factor)).
        self.zoom = max(1.0, min(5.0, factor))
        # Ejecuta self._clamp().
        self._clamp()
        # Publica la señal para sus receptores mediante self.zoom_changed.emit con self.zoom.
        self.zoom_changed.emit(self.zoom)
        # Ejecuta self.update().
        self.update()

    # Define _size sin parámetros de llamada.
    def _size(self):
        # Entra en esta rama si self.image.isNull().
        if self.image.isNull():
            # Devuelve (0.0, 0.0).
            return (0.0, 0.0)
        # Establece scale a partir de min(self.width() / self.image.width(), self.height() / self.image.height()) * sel…
        scale = min(self.width() / self.image.width(), self.height() / self.image.height()) * self.zoom
        # Devuelve (self.image.width() * scale, self.image.height() * scale).
        return self.image.width() * scale, self.image.height() * scale

    # Define _clamp sin parámetros de llamada.
    def _clamp(self):
        # Obtiene (width, height) con self._size().
        width, height = self._size()
        # Inicializa (dx, dy) con (max(0.0, (width - self.width()) / 2), max(0.0, (height - self.height()) / 2)).
        dx, dy = max(0.0, (width - self.width()) / 2), max(0.0, (height - self.height()) / 2)
        # Obtiene self.offset con QPointF(max(-dx, min(dx, self.offset.x())), max(-dy, min(dy, self.of…
        self.offset = QPointF(max(-dx, min(dx, self.offset.x())), max(-dy, min(dy, self.offset.y())))

    # Define paintEvent con el parámetro _event.
    def paintEvent(self, _event):
        # Obtiene painter con QPainter(self).
        painter = QPainter(self)
        # Ejecuta painter.setRenderHint(QPainter.Antialiasing).
        painter.setRenderHint(QPainter.Antialiasing)
        # Ejecuta painter.fillRect(self.rect(), QColor("#090911")).
        painter.fillRect(self.rect(), QColor("#090911"))
        # Entra en esta rama si not self.image.isNull().
        if not self.image.isNull():
            # Obtiene (width, height) con self._size().
            width, height = self._size()
            # Obtiene destination con QRectF((self.width() - width) / 2 + self.offset.x(), (self.height()…
            destination = QRectF((self.width() - width) / 2 + self.offset.x(),
                                 # Ejecuta (self.height() - height) / 2 + self.offset.y(), width, height).
                                 (self.height() - height) / 2 + self.offset.y(), width, height)
            # Ejecuta painter.setRenderHint(QPainter.SmoothPixmapTransform).
            painter.setRenderHint(QPainter.SmoothPixmapTransform)
            # Ejecuta painter.drawImage(destination, self.image).
            painter.drawImage(destination, self.image)
            # Sale de la función sin devolver un valor.
            return
        # Inicializa (cx, cy) con (self.width() / 2, self.height() / 2 - 55).
        cx, cy = self.width() / 2, self.height() / 2 - 55
        # Ejecuta painter.setPen(QPen(QColor("#44375f"), 2)).
        painter.setPen(QPen(QColor("#44375f"), 2))
        # Ejecuta painter.drawEllipse(QPointF(cx, cy), 43, 43).
        painter.drawEllipse(QPointF(cx, cy), 43, 43)
        # Ejecuta painter.setPen(QPen(QColor("#a985f2"), 3)).
        painter.setPen(QPen(QColor("#a985f2"), 3))
        # Ejecuta painter.drawEllipse(QPointF(cx, cy), 24, 24).
        painter.drawEllipse(QPointF(cx, cy), 24, 24)
        # Ejecuta painter.setPen(QPen(QColor("#66e8b3"), 3)).
        painter.setPen(QPen(QColor("#66e8b3"), 3))
        # Ejecuta painter.drawArc(QRectF(cx - 34, cy - 34, 68, 68), 35 * 16, 100 * 16).
        painter.drawArc(QRectF(cx - 34, cy - 34, 68, 68), 35 * 16, 100 * 16)
        # Ejecuta painter.setPen(QColor("#ece6ff")).
        painter.setPen(QColor("#ece6ff"))
        # Ejecuta painter.setFont(QFont("Segoe UI", 16, QFont.DemiBold)).
        painter.setFont(QFont("Segoe UI", 16, QFont.DemiBold))
        # Ejecuta painter.drawText(QRectF(18, cy + 58, self.width() - 36, 50), Qt.AlignHCenter | Qt.TextW…
        painter.drawText(QRectF(18, cy + 58, self.width() - 36, 50), Qt.AlignHCenter | Qt.TextWordWrap, self.message)
        # Ejecuta painter.setFont(QFont("Segoe UI", 10)).
        painter.setFont(QFont("Segoe UI", 10))
        # Ejecuta painter.setPen(QColor("#aaa2bd")).
        painter.setPen(QColor("#aaa2bd"))
        # Ejecuta painter.drawText(QRectF(18, cy + 111, self.width() - 36, 65), Qt.AlignHCenter | Qt.Text…
        painter.drawText(QRectF(18, cy + 111, self.width() - 36, 65), Qt.AlignHCenter | Qt.TextWordWrap, self.detail)

    # Define resizeEvent con el parámetro event.
    def resizeEvent(self, event):
        # Ejecuta self._clamp().
        self._clamp()
        # Ejecuta super().resizeEvent(event).
        super().resizeEvent(event)

    # Define wheelEvent con el parámetro event.
    def wheelEvent(self, event):
        # Entra en esta rama si not self.image.isNull().
        if not self.image.isNull():
            # Ejecuta self.set_zoom(self.zoom * (1.15 if event.angleDelta().y() > 0 else 1 / 1.15)).
            self.set_zoom(self.zoom * (1.15 if event.angleDelta().y() > 0 else 1 / 1.15))
        # Ejecuta event.accept().
        event.accept()

    # Define mousePressEvent con el parámetro event.
    def mousePressEvent(self, event):
        # Entra en esta rama si event.button() == Qt.LeftButton and self.zoom > 1.
        if event.button() == Qt.LeftButton and self.zoom > 1:
            # Obtiene self._drag con event.position().
            self._drag = event.position()
            # Ejecuta self.setCursor(Qt.ClosedHandCursor).
            self.setCursor(Qt.ClosedHandCursor)

    # Define mouseMoveEvent con el parámetro event.
    def mouseMoveEvent(self, event):
        # Entra en esta rama si self._drag is not None.
        if self._drag is not None:
            # Suma event.position() - self._drag al valor de self.offset.
            self.offset += event.position() - self._drag
            # Obtiene self._drag con event.position().
            self._drag = event.position()
            # Ejecuta self._clamp().
            self._clamp()
            # Ejecuta self.update().
            self.update()

    # Define mouseReleaseEvent con el parámetro event.
    def mouseReleaseEvent(self, event):
        # Establece self._drag a partir de None.
        self._drag = None
        # Ejecuta self.unsetCursor().
        self.unsetCursor()

    # Define mouseDoubleClickEvent con el parámetro event.
    def mouseDoubleClickEvent(self, event):
        # Ejecuta self.fit().
        self.fit()
