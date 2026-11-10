"""Image processing for the desktop, command-line viewer and benchmark."""

# Importa deque desde collections.
from collections import deque
# Importa monotonic desde time.
from time import monotonic

# Importa cv2.
import cv2

# Construye el mapa de datos MODES.
MODES = {
    # Asocia la clave "raw" con "RAW".
    "raw": "RAW",
    # Asocia la clave "gray" con "Grises".
    "gray": "Grises",
    # Asocia la clave "edges" con "Bordes".
    "edges": "Bordes",
    # Asocia la clave "motion" con "Movimiento".
    "motion": "Movimiento",
}


# Declara el tipo VisionPipeline para agrupar su estado y operaciones.
class VisionPipeline:
    # Inicializa la instancia con motion_area_percent.
    def __init__(self, motion_area_percent=0.25):
        # Establece self.motion_area_percent a partir de motion_area_percent.
        self.motion_area_percent = motion_area_percent
        # Establece self.mode (modo de procesamiento) a partir de None.
        self.mode = None
        # Establece self.previous_gray a partir de None.
        self.previous_gray = None
        # Establece self.last_detection a partir de None.
        self.last_detection = None
        # Obtiene self.detection_times con deque().
        self.detection_times = deque()
        # Establece self.total_detections a partir de 0.
        self.total_detections = 0

    # Define reset_reference sin parámetros de llamada.
    def reset_reference(self):
        # Establece self.previous_gray a partir de None.
        self.previous_gray = None

    # Aplica el modo de visión seleccionado a un fotograma BGR.
    def process(self, image, mode):
        # Entra en esta rama si mode != self.mode.
        if mode != self.mode:
            # Ejecuta self.reset_reference().
            self.reset_reference()
            # Establece self.mode (modo de procesamiento) a partir de mode.
            self.mode = mode
        # Guarda en now el resultado de monotonic, que lee un reloj monotónico.
        now = monotonic()
        # Repite mientras self.detection_times and self.detection_times[0] <= now - 60.
        while self.detection_times and self.detection_times[0] <= now - 60:
            # Ejecuta self.detection_times.popleft().
            self.detection_times.popleft()
        # Entra en esta rama si mode == 'raw'.
        if mode == "raw":
            # Devuelve image.
            return image
        # Guarda en gray el resultado de cv2.cvtColor, que convierte el espacio de color.
        gray = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)
        # Entra en esta rama si mode == 'gray'.
        if mode == "gray":
            # Devuelve cv2.cvtColor(gray, cv2.COLOR_GRAY2BGR).
            return cv2.cvtColor(gray, cv2.COLOR_GRAY2BGR)
        # Entra en esta rama si mode == 'edges'.
        if mode == "edges":
            # Guarda en edges el resultado de cv2.Canny, que detecta bordes con Canny.
            edges = cv2.Canny(cv2.GaussianBlur(gray, (5, 5), 0), 60, 150)
            # Devuelve cv2.cvtColor(edges, cv2.COLOR_GRAY2BGR).
            return cv2.cvtColor(edges, cv2.COLOR_GRAY2BGR)
        # Entra en esta rama si mode != 'motion'.
        if mode != "motion":
            # Interrumpe la operación con ValueError('Modo de visión desconocido: ' + str(mode)).
            raise ValueError("Modo de visión desconocido: " + str(mode))

        # Guarda en current el resultado de cv2.GaussianBlur, que suaviza la imagen con un filtro gaussiano.
        current = cv2.GaussianBlur(gray, (5, 5), 0)
        # Inicializa (previous, self.previous_gray) con (self.previous_gray, current).
        previous, self.previous_gray = self.previous_gray, current
        # Evita comparar la primera imagen o dos fotogramas de dimensiones distintas.
        if previous is None or previous.shape != current.shape:
            # Devuelve image.
            return image
        # Calcula cuánto cambió cada píxel entre los dos fotogramas suavizados.
        difference = cv2.absdiff(previous, current)
        # Marca en blanco las diferencias de intensidad que superan 25.
        _, mask = cv2.threshold(difference, 25, 255, cv2.THRESH_BINARY)
        # Une regiones cercanas de movimiento mediante dos dilataciones.
        mask = cv2.dilate(mask, None, iterations=2)
        # Guarda en (regions, _) el resultado de cv2.findContours, que encuentra los contornos de las regiones.
        regions, _ = cv2.findContours(mask, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
        # Obtiene area_limit con max(20, current.size * self.motion_area_percent / 100).
        area_limit = max(20, current.size * self.motion_area_percent / 100)
        # Calcula boxes a partir de regions.
        boxes = [cv2.boundingRect(region) for region in regions if cv2.contourArea(region) >= area_limit]
        # Entra en esta rama si boxes.
        if boxes:
            # Entra en esta rama si self.last_detection is None or now - self.last_detection >= 1.0.
            if self.last_detection is None or now - self.last_detection >= 1.0:
                # Agrega un elemento al final mediante self.detection_times.append con now.
                self.detection_times.append(now)
                # Suma 1 al valor de self.total_detections.
                self.total_detections += 1
                # Establece self.last_detection a partir de now.
                self.last_detection = now
            # Recorre boxes y toma cada elemento como (x, y, width, height).
            for x, y, width, height in boxes:
                # Dibuja un rectángulo sobre la imagen mediante cv2.rectangle con image, (x, y).
                cv2.rectangle(image, (x, y), (x + width - 1, y + height - 1), (120, 232, 102), 2)
        # Devuelve image.
        return image
