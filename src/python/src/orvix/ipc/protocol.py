"""Versioned ORVIX shared-memory wire protocol."""

# Importa annotations desde __future__.
from __future__ import annotations

# Importa struct.
import struct
# Importa dataclass desde dataclasses.
from dataclasses import dataclass
# Importa Optional desde typing.
from typing import Optional

# Establece MAGIC a partir de b'ORVX'.
MAGIC = b"ORVX"
# Establece PROTOCOL_VERSION a partir de 1.
PROTOCOL_VERSION = 1
# Establece GLOBAL_HEADER_SIZE a partir de 128.
GLOBAL_HEADER_SIZE = 128
# Establece SLOT_HEADER_SIZE a partir de 64.
SLOT_HEADER_SIZE = 64
# Establece DEFAULT_SHARED_MEMORY_NAME a partir de 'orvix-camera-v1'.
DEFAULT_SHARED_MEMORY_NAME = "orvix-camera-v1"

# Establece PRODUCER_INITIALIZING a partir de 0.
PRODUCER_INITIALIZING = 0
# Establece PRODUCER_RUNNING a partir de 1.
PRODUCER_RUNNING = 1
# Establece PRODUCER_STOPPED a partir de 2.
PRODUCER_STOPPED = 2
# Establece PRODUCER_FAILED a partir de 3.
PRODUCER_FAILED = 3

# Construye el mapa de datos PIXEL_FORMATS.
PIXEL_FORMATS = {
    # Aporta 0: "UNKNOWN" como elemento de la lista.
    0: "UNKNOWN",
    # Aporta 1: "NV12" como elemento de la lista.
    1: "NV12",
    # Aporta 2: "YUY2" como elemento de la lista.
    2: "YUY2",
    # Aporta 3: "UYVY" como elemento de la lista.
    3: "UYVY",
    # Aporta 4: "I420" como elemento de la lista.
    4: "I420",
    # Aporta 5: "YV12" como elemento de la lista.
    5: "YV12",
    # Aporta 6: "RGB24" como elemento de la lista.
    6: "RGB24",
    # Aporta 7: "BGRA32" como elemento de la lista.
    7: "BGRA32",
    # Aporta 8: "ARGB32" como elemento de la lista.
    8: "ARGB32",
    # Aporta 9: "MJPG" como elemento de la lista.
    9: "MJPG",
    # Aporta 10: "H264" como elemento de la lista.
    10: "H264",
}


# Declara el tipo ProtocolError que hereda de RuntimeError.
class ProtocolError(RuntimeError):
    """Raised when a mapping does not satisfy the ORVIX IPC contract."""


# Aplica el decorador dataclass(frozen=True) a la definición siguiente.
@dataclass(frozen=True)
# Declara el tipo SharedHeader para agrupar su estado y operaciones.
class SharedHeader:
    # Declara el tipo de total_size.
    total_size: int
    # Declara el tipo de slot_count (cantidad de posiciones del anillo).
    slot_count: int
    # Declara el tipo de slot_header_size.
    slot_header_size: int
    # Declara el tipo de payload_capacity.
    payload_capacity: int
    # Declara el tipo de slot_stride.
    slot_stride: int
    # Declara el tipo de width (ancho en píxeles).
    width: int
    # Declara el tipo de height (alto en píxeles).
    height: int
    # Declara el tipo de frame_stride.
    frame_stride: int
    # Declara el tipo de pixel_format (formato de píxeles).
    pixel_format: str
    # Declara el tipo de fps_numerator.
    fps_numerator: int
    # Declara el tipo de fps_denominator.
    fps_denominator: int
    # Declara el tipo de created_timestamp_100ns.
    created_timestamp_100ns: int
    # Declara el tipo de producer_pid.
    producer_pid: int
    # Declara el tipo de producer_state.
    producer_state: int
    # Declara el tipo de published_slot (slot publicado).
    published_slot: int
    # Declara el tipo de published_sequence (última secuencia publicada).
    published_sequence: int
    # Declara el tipo de consumed_sequence.
    consumed_sequence: int
    # Declara el tipo de overwritten_frames.
    overwritten_frames: int
    # Declara el tipo de producer_fps.
    producer_fps: float

    # Aplica el decorador property a la definición siguiente.
    @property
    # Define buffer_utilization sin parámetros de llamada.
    def buffer_utilization(self) -> float:
        # Obtiene pending con max(0, self.published_sequence - self.consumed_sequence).
        pending = max(0, self.published_sequence - self.consumed_sequence)
        # Devuelve min(pending, self.slot_count) / self.slot_count.
        return min(pending, self.slot_count) / self.slot_count


# Aplica el decorador dataclass(frozen=True) a la definición siguiente.
@dataclass(frozen=True)
# Declara el tipo SharedFrame para agrupar su estado y operaciones.
class SharedFrame:
    # Declara el tipo de sequence (secuencia del fotograma).
    sequence: int
    # Declara el tipo de timestamp_100ns (marca de tiempo en unidades de 100 ns).
    timestamp_100ns: int
    # Declara el tipo de width (ancho en píxeles).
    width: int
    # Declara el tipo de height (alto en píxeles).
    height: int
    # Declara el tipo de stride (bytes entre filas de imagen).
    stride: int
    # Declara el tipo de pixel_format (formato de píxeles).
    pixel_format: str
    # Declara el tipo de payload (carga útil del fotograma).
    payload: bytes


# Define _u16 con los parámetros buffer, offset.
def _u16(buffer: memoryview, offset: int) -> int:
    # Devuelve struct.unpack_from('<H', buffer, offset)[0].
    return struct.unpack_from("<H", buffer, offset)[0]


# Define _u32 con los parámetros buffer, offset.
def _u32(buffer: memoryview, offset: int) -> int:
    # Devuelve struct.unpack_from('<I', buffer, offset)[0].
    return struct.unpack_from("<I", buffer, offset)[0]


# Define _u64 con los parámetros buffer, offset.
def _u64(buffer: memoryview, offset: int) -> int:
    # Devuelve struct.unpack_from('<Q', buffer, offset)[0].
    return struct.unpack_from("<Q", buffer, offset)[0]


# Define _i64 con los parámetros buffer, offset.
def _i64(buffer: memoryview, offset: int) -> int:
    # Devuelve struct.unpack_from('<q', buffer, offset)[0].
    return struct.unpack_from("<q", buffer, offset)[0]


# Define read_header con el parámetro buffer.
def read_header(buffer: memoryview) -> SharedHeader:
    """Validate and decode the fixed 128-byte global header."""

    # Entra en esta rama si len(buffer) < GLOBAL_HEADER_SIZE.
    if len(buffer) < GLOBAL_HEADER_SIZE:
        # Interrumpe la operación con ProtocolError('Shared memory is smaller than the ORVIX header.').
        raise ProtocolError("Shared memory is smaller than the ORVIX header.")
    # Entra en esta rama si bytes(buffer[0:4]) != MAGIC.
    if bytes(buffer[0:4]) != MAGIC:
        # Interrumpe la operación con ProtocolError('Shared memory does not contain ORVIX magic bytes.').
        raise ProtocolError("Shared memory does not contain ORVIX magic bytes.")
    # Obtiene version con _u16(buffer, 4).
    version = _u16(buffer, 4)
    # Entra en esta rama si version != PROTOCOL_VERSION.
    if version != PROTOCOL_VERSION:
        # Interrumpe la operación con ProtocolError('Unsupported ORVIX protocol version: {}.'.format(version)).
        raise ProtocolError(
            # Ejecuta "Unsupported ORVIX protocol version: {}.".format(version).
            "Unsupported ORVIX protocol version: {}.".format(version)
        )
    # Obtiene header_size con _u16(buffer, 6).
    header_size = _u16(buffer, 6)
    # Entra en esta rama si header_size != GLOBAL_HEADER_SIZE.
    if header_size != GLOBAL_HEADER_SIZE:
        # Interrumpe la operación con ProtocolError('Invalid ORVIX global-header size.').
        raise ProtocolError("Invalid ORVIX global-header size.")

    # Obtiene total_size con _u64(buffer, 8).
    total_size = _u64(buffer, 8)
    # Obtiene slot_count (cantidad de posiciones del anillo) con _u32(buffer, 16).
    slot_count = _u32(buffer, 16)
    # Obtiene slot_header_size con _u32(buffer, 20).
    slot_header_size = _u32(buffer, 20)
    # Obtiene payload_capacity con _u64(buffer, 24).
    payload_capacity = _u64(buffer, 24)
    # Obtiene slot_stride con _u64(buffer, 32).
    slot_stride = _u64(buffer, 32)
    # Entra en esta rama si slot_count < 2 or slot_header_size != SLOT_HEADER_SIZE.
    if slot_count < 2 or slot_header_size != SLOT_HEADER_SIZE:
        # Interrumpe la operación con ProtocolError('Invalid ORVIX ring-buffer geometry.').
        raise ProtocolError("Invalid ORVIX ring-buffer geometry.")
    # Establece expected a partir de GLOBAL_HEADER_SIZE + slot_count * slot_stride.
    expected = GLOBAL_HEADER_SIZE + slot_count * slot_stride
    # Entra en esta rama si total_size != expected or total_size > len(buffer).
    if total_size != expected or total_size > len(buffer):
        # Interrumpe la operación con ProtocolError('ORVIX mapping size does not match its header.').
        raise ProtocolError("ORVIX mapping size does not match its header.")

    # Obtiene format_code con _u32(buffer, 52).
    format_code = _u32(buffer, 52)
    # Devuelve SharedHeader(total_size=total_size, slot_count=slot_count, slot_header_size=slot_…
    return SharedHeader(
        # Indica total_size con valor total_size.
        total_size=total_size,
        # Indica slot_count (cantidad de posiciones del anillo) con valor slot_count.
        slot_count=slot_count,
        # Indica slot_header_size con valor slot_header_size.
        slot_header_size=slot_header_size,
        # Indica payload_capacity con valor payload_capacity.
        payload_capacity=payload_capacity,
        # Indica slot_stride con valor slot_stride.
        slot_stride=slot_stride,
        # Indica width (ancho en píxeles) con valor _u32(buffer, 40).
        width=_u32(buffer, 40),
        # Indica height (alto en píxeles) con valor _u32(buffer, 44).
        height=_u32(buffer, 44),
        # Indica frame_stride con valor _u32(buffer, 48).
        frame_stride=_u32(buffer, 48),
        # Indica pixel_format (formato de píxeles) con valor PIXEL_FORMATS.get(format_code, "UNKNOWN").
        pixel_format=PIXEL_FORMATS.get(format_code, "UNKNOWN"),
        # Indica fps_numerator con valor _u32(buffer, 56).
        fps_numerator=_u32(buffer, 56),
        # Indica fps_denominator con valor _u32(buffer, 60).
        fps_denominator=_u32(buffer, 60),
        # Indica created_timestamp_100ns con valor _i64(buffer, 64).
        created_timestamp_100ns=_i64(buffer, 64),
        # Indica producer_pid con valor _u64(buffer, 72).
        producer_pid=_u64(buffer, 72),
        # Indica producer_state con valor _u32(buffer, 80).
        producer_state=_u32(buffer, 80),
        # Indica published_slot (slot publicado) con valor _u32(buffer, 84).
        published_slot=_u32(buffer, 84),
        # Indica published_sequence (última secuencia publicada) con valor _u64(buffer, 88).
        published_sequence=_u64(buffer, 88),
        # Indica consumed_sequence con valor _u64(buffer, 96).
        consumed_sequence=_u64(buffer, 96),
        # Indica overwritten_frames con valor _u64(buffer, 104).
        overwritten_frames=_u64(buffer, 104),
        # Indica producer_fps con valor _u64(buffer, 112) / 1000.0.
        producer_fps=_u64(buffer, 112) / 1000.0,
    )


# Lee el último fotograma completo del anillo y reintenta si cambia durante la copia.
def read_latest_frame(
    # Aporta buffer: memoryview como elemento de la lista.
    buffer: memoryview,
    # Aporta retries: int = 20 como elemento de la lista.
    retries: int = 20,
# Completa la expresión y encadena -> Optional[SharedFrame]:.
) -> Optional[SharedFrame]:
    """Copy the newest complete frame using the slot seqlock."""

    # Entra en esta rama si retries < 1.
    if retries < 1:
        # Interrumpe la operación con ValueError('retries must be positive').
        raise ValueError("retries must be positive")
    # Recorre range(retries) y toma cada elemento como _.
    for _ in range(retries):
        # Lee la cabecera global para localizar el último slot publicado.
        header = read_header(buffer)
        # Comprueba si el productor aún no ha publicado un fotograma.
        if header.published_sequence == 0:
            # Devuelve None.
            return None
        # Entra en esta rama si header.published_slot >= header.slot_count.
        if header.published_slot >= header.slot_count:
            # Interrumpe la operación con ProtocolError('Published slot is outside the ORVIX ring.').
            raise ProtocolError("Published slot is outside the ORVIX ring.")

        # Calcula el desplazamiento del slot dentro de la región compartida.
        base = GLOBAL_HEADER_SIZE + header.published_slot * header.slot_stride
        # Lee la generación del slot antes de copiar su contenido.
        generation_before = _u64(buffer, base)
        # Una generación impar indica que el productor está escribiendo el slot.
        if generation_before & 1:
            # Omite lo restante y pasa a la siguiente iteración.
            continue

        # Obtiene sequence (secuencia del fotograma) con _u64(buffer, base + 8).
        sequence = _u64(buffer, base + 8)
        # Obtiene timestamp (marca de tiempo) con _i64(buffer, base + 16).
        timestamp = _i64(buffer, base + 16)
        # Obtiene payload_size (tamaño de la carga útil) con _u64(buffer, base + 24).
        payload_size = _u64(buffer, base + 24)
        # Obtiene width (ancho en píxeles) con _u32(buffer, base + 32).
        width = _u32(buffer, base + 32)
        # Obtiene height (alto en píxeles) con _u32(buffer, base + 36).
        height = _u32(buffer, base + 36)
        # Obtiene stride (bytes entre filas de imagen) con _u32(buffer, base + 40).
        stride = _u32(buffer, base + 40)
        # Obtiene format_code con _u32(buffer, base + 44).
        format_code = _u32(buffer, base + 44)
        # Obtiene state (estado) con _u32(buffer, base + 48).
        state = _u32(buffer, base + 48)
        # Entra en esta rama si payload_size > header.payload_capacity.
        if payload_size > header.payload_capacity:
            # Interrumpe la operación con ProtocolError('Frame payload exceeds its declared capacity.').
            raise ProtocolError("Frame payload exceeds its declared capacity.")
        # Entra en esta rama si state != 2.
        if state != 2:
            # Omite lo restante y pasa a la siguiente iteración.
            continue

        # Establece start a partir de base + SLOT_HEADER_SIZE.
        start = base + SLOT_HEADER_SIZE
        # Copia exactamente los bytes de imagen declarados por el slot.
        payload = bytes(buffer[start : start + payload_size])
        # Vuelve a leer la generación para detectar escrituras durante la copia.
        generation_after = _u64(buffer, base)
        # Entra en esta rama si generation_before == generation_after and (not generation_after & 1) and (sequence == h…
        if (
            # Exige que el productor no haya cambiado la generación durante la copia.
            generation_before == generation_after
            # Comprueba que el slot no esté marcado como una escritura en curso.
            and not generation_after & 1
            # Exige que la secuencia copiada coincida con la publicada en la cabecera.
            and sequence == header.published_sequence
        # Completa la expresión y encadena :.
        ):
            # Devuelve SharedFrame(sequence=sequence, timestamp_100ns=timestamp, width=width, height=hei…
            return SharedFrame(
                # Indica sequence (secuencia del fotograma) con valor sequence.
                sequence=sequence,
                # Indica timestamp_100ns (marca de tiempo en unidades de 100 ns) con valor timestamp.
                timestamp_100ns=timestamp,
                # Indica width (ancho en píxeles) con valor width.
                width=width,
                # Indica height (alto en píxeles) con valor height.
                height=height,
                # Indica stride (bytes entre filas de imagen) con valor stride.
                stride=stride,
                # Indica pixel_format (formato de píxeles) con valor PIXEL_FORMATS.get(format_code, "UNKNOWN").
                pixel_format=PIXEL_FORMATS.get(format_code, "UNKNOWN"),
                # Indica payload (carga útil del fotograma) con valor payload.
                payload=payload,
            )
    # Devuelve None.
    return None


# Define acknowledge_frame con los parámetros buffer, sequence.
def acknowledge_frame(buffer: memoryview, sequence: int) -> None:
    """Publish the last sequence copied by the Python consumer."""

    # Entra en esta rama si sequence < 0.
    if sequence < 0:
        # Interrumpe la operación con ValueError('sequence cannot be negative').
        raise ValueError("sequence cannot be negative")
    # Ejecuta struct.pack_into("<Q", buffer, 96, sequence).
    struct.pack_into("<Q", buffer, 96, sequence)
