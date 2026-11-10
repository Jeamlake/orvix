"""Image processing for the desktop, command-line viewer and benchmark."""

from collections import deque
from time import monotonic

import cv2

MODES = {
    "raw": "RAW",
    "gray": "Grises",
    "edges": "Bordes",
    "motion": "Movimiento",
}


class VisionPipeline:
    def __init__(self, motion_area_percent=0.25):
        self.motion_area_percent = motion_area_percent
        self.mode = None
        self.previous_gray = None
        self.last_detection = None
        self.detection_times = deque()
        self.total_detections = 0

    def reset_reference(self):
        self.previous_gray = None

    def process(self, image, mode):
        if mode != self.mode:
            self.reset_reference()
            self.mode = mode
        now = monotonic()
        while self.detection_times and self.detection_times[0] <= now - 60:
            self.detection_times.popleft()
        if mode == "raw":
            return image
        gray = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)
        if mode == "gray":
            return cv2.cvtColor(gray, cv2.COLOR_GRAY2BGR)
        if mode == "edges":
            edges = cv2.Canny(cv2.GaussianBlur(gray, (5, 5), 0), 60, 150)
            return cv2.cvtColor(edges, cv2.COLOR_GRAY2BGR)
        if mode != "motion":
            raise ValueError("Modo de visión desconocido: " + str(mode))

        current = cv2.GaussianBlur(gray, (5, 5), 0)
        previous, self.previous_gray = self.previous_gray, current
        if previous is None or previous.shape != current.shape:
            return image
        difference = cv2.absdiff(previous, current)
        _, mask = cv2.threshold(difference, 25, 255, cv2.THRESH_BINARY)
        mask = cv2.dilate(mask, None, iterations=2)
        regions, _ = cv2.findContours(mask, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
        area_limit = max(20, current.size * self.motion_area_percent / 100)
        boxes = [cv2.boundingRect(region) for region in regions if cv2.contourArea(region) >= area_limit]
        if boxes:
            if self.last_detection is None or now - self.last_detection >= 1.0:
                self.detection_times.append(now)
                self.total_detections += 1
                self.last_detection = now
            for x, y, width, height in boxes:
                cv2.rectangle(image, (x, y), (x + width - 1, y + height - 1), (120, 232, 102), 2)
        return image
