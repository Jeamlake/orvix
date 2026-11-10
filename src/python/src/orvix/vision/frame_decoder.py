"""Decode native ORVIX frame payloads into OpenCV BGR arrays."""

from __future__ import annotations

import cv2
import numpy as np

from orvix.ipc.protocol import SharedFrame


class FrameDecodeError(RuntimeError):
    """Raised when a native payload cannot be converted for display."""


def _rows(payload: bytes, height: int, stride: int) -> np.ndarray:
    needed = height * stride
    if stride <= 0 or len(payload) < needed:
        raise FrameDecodeError("Frame payload is smaller than its row geometry.")
    return np.frombuffer(payload, dtype=np.uint8, count=needed).reshape(
        height, stride
    )


def decode_bgr(frame: SharedFrame) -> np.ndarray:
    """Return an owned BGR image suitable for cv2.imshow."""

    width, height, stride = frame.width, frame.height, frame.stride
    if width <= 0 or height <= 0:
        raise FrameDecodeError("Frame dimensions must be positive.")

    if frame.pixel_format == "NV12":
        if width % 2 or height % 2 or stride < width:
            raise FrameDecodeError("NV12 requires even dimensions and stride >= width.")
        y_size = stride * height
        needed = y_size + stride * (height // 2)
        if len(frame.payload) < needed:
            raise FrameDecodeError("NV12 payload is incomplete.")
        packed = np.empty((height + height // 2, width), dtype=np.uint8)
        packed[:height] = _rows(frame.payload, height, stride)[:, :width]
        uv = np.frombuffer(
            frame.payload,
            dtype=np.uint8,
            count=stride * (height // 2),
            offset=y_size,
        ).reshape(height // 2, stride)
        packed[height:] = uv[:, :width]
        return cv2.cvtColor(packed, cv2.COLOR_YUV2BGR_NV12)

    if frame.pixel_format in ("YUY2", "UYVY"):
        row_bytes = width * 2
        if stride < row_bytes:
            raise FrameDecodeError("Packed YUV stride is too small.")
        packed = _rows(frame.payload, height, stride)[:, :row_bytes]
        packed = np.ascontiguousarray(packed.reshape(height, width, 2))
        conversion = (
            cv2.COLOR_YUV2BGR_YUY2
            if frame.pixel_format == "YUY2"
            else cv2.COLOR_YUV2BGR_UYVY
        )
        return cv2.cvtColor(packed, conversion)

    if frame.pixel_format == "RGB24":
        row_bytes = width * 3
        if stride < row_bytes:
            raise FrameDecodeError("RGB24 stride is too small.")
        rgb = _rows(frame.payload, height, stride)[:, :row_bytes]
        rgb = np.ascontiguousarray(rgb.reshape(height, width, 3))
        return cv2.cvtColor(rgb, cv2.COLOR_RGB2BGR)

    if frame.pixel_format in ("BGRA32", "ARGB32"):
        row_bytes = width * 4
        if stride < row_bytes:
            raise FrameDecodeError("32-bit RGB stride is too small.")
        pixels = _rows(frame.payload, height, stride)[:, :row_bytes]
        pixels = np.ascontiguousarray(pixels.reshape(height, width, 4))
        if frame.pixel_format == "BGRA32":
            return cv2.cvtColor(pixels, cv2.COLOR_BGRA2BGR)
        return np.ascontiguousarray(pixels[:, :, [3, 2, 1]])

    if frame.pixel_format in ("I420", "YV12"):
        if width % 2 or height % 2 or stride != width:
            raise FrameDecodeError("Planar YUV requires packed even dimensions.")
        needed = width * height * 3 // 2
        if len(frame.payload) < needed:
            raise FrameDecodeError("Planar YUV payload is incomplete.")
        packed = np.frombuffer(
            frame.payload, dtype=np.uint8, count=needed
        ).reshape(height * 3 // 2, width)
        conversion = (
            cv2.COLOR_YUV2BGR_I420
            if frame.pixel_format == "I420"
            else cv2.COLOR_YUV2BGR_YV12
        )
        return cv2.cvtColor(packed, conversion)

    if frame.pixel_format == "MJPG":
        encoded = np.frombuffer(frame.payload, dtype=np.uint8)
        decoded = cv2.imdecode(encoded, cv2.IMREAD_COLOR)
        if decoded is None:
            raise FrameDecodeError("OpenCV could not decode the MJPG frame.")
        return decoded

    raise FrameDecodeError(
        "Pixel format '{}' is not displayable.".format(frame.pixel_format)
    )
