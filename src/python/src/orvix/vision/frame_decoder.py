"""Decode native ORVIX frame payloads into OpenCV BGR arrays."""

# Importa annotations desde __future__.
from __future__ import annotations

# Importa cv2.
import cv2
# Importa numpy como np.
import numpy as np

# Importa SharedFrame desde orvix.ipc.protocol.
from orvix.ipc.protocol import SharedFrame


# Declara el tipo FrameDecodeError que hereda de RuntimeError.
class FrameDecodeError(RuntimeError):
    """Raised when a native payload cannot be converted for display."""


# Define _rows con los parámetros payload, height, stride.
def _rows(payload: bytes, height: int, stride: int) -> np.ndarray:
    # Establece needed a partir de height * stride.
    needed = height * stride
    # Entra en esta rama si stride <= 0 or len(payload) < needed.
    if stride <= 0 or len(payload) < needed:
        # Interrumpe la operación con FrameDecodeError('Frame payload is smaller than its row geometry.').
        raise FrameDecodeError("Frame payload is smaller than its row geometry.")
    # Devuelve np.frombuffer(payload, dtype=np.uint8, count=needed).reshape(height, stride).
    return np.frombuffer(payload, dtype=np.uint8, count=needed).reshape(
        # Incorpora height, stride en esta operación.
        height, stride
    )


# Define decode_bgr, que decodifica el fotograma como imagen BGR.
def decode_bgr(frame: SharedFrame) -> np.ndarray:
    """Return an owned BGR image suitable for cv2.imshow."""

    # Inicializa (width, height, stride) con (frame.width, frame.height, frame.stride).
    width, height, stride = frame.width, frame.height, frame.stride
    # Entra en esta rama si width <= 0 or height <= 0.
    if width <= 0 or height <= 0:
        # Interrumpe la operación con FrameDecodeError('Frame dimensions must be positive.').
        raise FrameDecodeError("Frame dimensions must be positive.")

    # Entra en esta rama si frame.pixel_format == 'NV12'.
    if frame.pixel_format == "NV12":
        # Entra en esta rama si width % 2 or height % 2 or stride < width.
        if width % 2 or height % 2 or stride < width:
            # Interrumpe la operación con FrameDecodeError('NV12 requires even dimensions and stride >= width.').
            raise FrameDecodeError("NV12 requires even dimensions and stride >= width.")
        # Establece y_size a partir de stride * height.
        y_size = stride * height
        # Establece needed a partir de y_size + stride * (height // 2).
        needed = y_size + stride * (height // 2)
        # Entra en esta rama si len(frame.payload) < needed.
        if len(frame.payload) < needed:
            # Interrumpe la operación con FrameDecodeError('NV12 payload is incomplete.').
            raise FrameDecodeError("NV12 payload is incomplete.")
        # Obtiene packed con np.empty((height + height // 2, width), dtype=np.uint8).
        packed = np.empty((height + height // 2, width), dtype=np.uint8)
        # Establece packed[:height] a partir de _rows(frame.payload, height, stride)[:, :width].
        packed[:height] = _rows(frame.payload, height, stride)[:, :width]
        # Obtiene uv con np.frombuffer(frame.payload, dtype=np.uint8, count=stride * (height…
        uv = np.frombuffer(
            # Aporta frame.payload como elemento de la lista.
            frame.payload,
            # Indica dtype con valor np.uint8.
            dtype=np.uint8,
            # Indica count con valor stride * (height // 2).
            count=stride * (height // 2),
            # Indica offset con valor y_size.
            offset=y_size,
        # Completa la expresión y encadena reshape(height // 2, stride).
        ).reshape(height // 2, stride)
        # Establece packed[height:] a partir de uv[:, :width].
        packed[height:] = uv[:, :width]
        # Devuelve cv2.cvtColor(packed, cv2.COLOR_YUV2BGR_NV12).
        return cv2.cvtColor(packed, cv2.COLOR_YUV2BGR_NV12)

    # Entra en esta rama si frame.pixel_format in ('YUY2', 'UYVY').
    if frame.pixel_format in ("YUY2", "UYVY"):
        # Establece row_bytes a partir de width * 2.
        row_bytes = width * 2
        # Entra en esta rama si stride < row_bytes.
        if stride < row_bytes:
            # Interrumpe la operación con FrameDecodeError('Packed YUV stride is too small.').
            raise FrameDecodeError("Packed YUV stride is too small.")
        # Establece packed a partir de _rows(frame.payload, height, stride)[:, :row_bytes].
        packed = _rows(frame.payload, height, stride)[:, :row_bytes]
        # Obtiene packed con np.ascontiguousarray(packed.reshape(height, width, 2)).
        packed = np.ascontiguousarray(packed.reshape(height, width, 2))
        # Establece conversion a partir de cv2.COLOR_YUV2BGR_YUY2 if frame.pixel_format == 'YUY2' else cv2.COLOR_YUV2BGR_UYVY.
        conversion = (
            # Incorpora cv2.COLOR_YUV2BGR_YUY2 en esta operación.
            cv2.COLOR_YUV2BGR_YUY2
            # Incorpora if frame.pixel_format == "YUY2" como condición o recorrido de la expresión.
            if frame.pixel_format == "YUY2"
            # Incorpora else cv2.COLOR_YUV2BGR_UYVY en esta operación.
            else cv2.COLOR_YUV2BGR_UYVY
        )
        # Devuelve cv2.cvtColor(packed, conversion).
        return cv2.cvtColor(packed, conversion)

    # Entra en esta rama si frame.pixel_format == 'RGB24'.
    if frame.pixel_format == "RGB24":
        # Establece row_bytes a partir de width * 3.
        row_bytes = width * 3
        # Entra en esta rama si stride < row_bytes.
        if stride < row_bytes:
            # Interrumpe la operación con FrameDecodeError('RGB24 stride is too small.').
            raise FrameDecodeError("RGB24 stride is too small.")
        # Establece rgb a partir de _rows(frame.payload, height, stride)[:, :row_bytes].
        rgb = _rows(frame.payload, height, stride)[:, :row_bytes]
        # Obtiene rgb con np.ascontiguousarray(rgb.reshape(height, width, 3)).
        rgb = np.ascontiguousarray(rgb.reshape(height, width, 3))
        # Devuelve cv2.cvtColor(rgb, cv2.COLOR_RGB2BGR).
        return cv2.cvtColor(rgb, cv2.COLOR_RGB2BGR)

    # Entra en esta rama si frame.pixel_format in ('BGRA32', 'ARGB32').
    if frame.pixel_format in ("BGRA32", "ARGB32"):
        # Establece row_bytes a partir de width * 4.
        row_bytes = width * 4
        # Entra en esta rama si stride < row_bytes.
        if stride < row_bytes:
            # Interrumpe la operación con FrameDecodeError('32-bit RGB stride is too small.').
            raise FrameDecodeError("32-bit RGB stride is too small.")
        # Establece pixels a partir de _rows(frame.payload, height, stride)[:, :row_bytes].
        pixels = _rows(frame.payload, height, stride)[:, :row_bytes]
        # Obtiene pixels con np.ascontiguousarray(pixels.reshape(height, width, 4)).
        pixels = np.ascontiguousarray(pixels.reshape(height, width, 4))
        # Entra en esta rama si frame.pixel_format == 'BGRA32'.
        if frame.pixel_format == "BGRA32":
            # Devuelve cv2.cvtColor(pixels, cv2.COLOR_BGRA2BGR).
            return cv2.cvtColor(pixels, cv2.COLOR_BGRA2BGR)
        # Devuelve np.ascontiguousarray(pixels[:, :, [3, 2, 1]]).
        return np.ascontiguousarray(pixels[:, :, [3, 2, 1]])

    # Entra en esta rama si frame.pixel_format in ('I420', 'YV12').
    if frame.pixel_format in ("I420", "YV12"):
        # Entra en esta rama si width % 2 or height % 2 or stride != width.
        if width % 2 or height % 2 or stride != width:
            # Interrumpe la operación con FrameDecodeError('Planar YUV requires packed even dimensions.').
            raise FrameDecodeError("Planar YUV requires packed even dimensions.")
        # Establece needed a partir de width * height * 3 // 2.
        needed = width * height * 3 // 2
        # Entra en esta rama si len(frame.payload) < needed.
        if len(frame.payload) < needed:
            # Interrumpe la operación con FrameDecodeError('Planar YUV payload is incomplete.').
            raise FrameDecodeError("Planar YUV payload is incomplete.")
        # Obtiene packed con np.frombuffer(frame.payload, dtype=np.uint8, count=needed).reshape(h…
        packed = np.frombuffer(
            # Incorpora frame.payload, dtype=np.uint8, count=needed en esta operación.
            frame.payload, dtype=np.uint8, count=needed
        # Completa la expresión y encadena reshape(height * 3 // 2, width).
        ).reshape(height * 3 // 2, width)
        # Establece conversion a partir de cv2.COLOR_YUV2BGR_I420 if frame.pixel_format == 'I420' else cv2.COLOR_YUV2BGR_YV12.
        conversion = (
            # Incorpora cv2.COLOR_YUV2BGR_I420 en esta operación.
            cv2.COLOR_YUV2BGR_I420
            # Incorpora if frame.pixel_format == "I420" como condición o recorrido de la expresión.
            if frame.pixel_format == "I420"
            # Incorpora else cv2.COLOR_YUV2BGR_YV12 en esta operación.
            else cv2.COLOR_YUV2BGR_YV12
        )
        # Devuelve cv2.cvtColor(packed, conversion).
        return cv2.cvtColor(packed, conversion)

    # Entra en esta rama si frame.pixel_format == 'MJPG'.
    if frame.pixel_format == "MJPG":
        # Obtiene encoded con np.frombuffer(frame.payload, dtype=np.uint8).
        encoded = np.frombuffer(frame.payload, dtype=np.uint8)
        # Obtiene decoded con cv2.imdecode(encoded, cv2.IMREAD_COLOR).
        decoded = cv2.imdecode(encoded, cv2.IMREAD_COLOR)
        # Entra en esta rama si decoded is None.
        if decoded is None:
            # Interrumpe la operación con FrameDecodeError('OpenCV could not decode the MJPG frame.').
            raise FrameDecodeError("OpenCV could not decode the MJPG frame.")
        # Devuelve decoded.
        return decoded

    # Interrumpe la operación con FrameDecodeError("Pixel format '{}' is not displayable.".format(frame.pixel_format)).
    raise FrameDecodeError(
        # Ejecuta "Pixel format '{}' is not displayable.".format(frame.pixel_format).
        "Pixel format '{}' is not displayable.".format(frame.pixel_format)
    )
