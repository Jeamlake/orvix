# Importa struct.
import struct

# Importa GLOBAL_HEADER_SIZE, SLOT_HEADER_SIZE, acknowledge_frame, read_header, read_latest_frame desde orvix.ipc.protocol.
from orvix.ipc.protocol import (
    # Aporta GLOBAL_HEADER_SIZE como elemento de la lista.
    GLOBAL_HEADER_SIZE,
    # Aporta SLOT_HEADER_SIZE como elemento de la lista.
    SLOT_HEADER_SIZE,
    # Aporta acknowledge_frame como elemento de la lista.
    acknowledge_frame,
    # Aporta read_header como elemento de la lista.
    read_header,
    # Aporta read_latest_frame como elemento de la lista.
    read_latest_frame,
)


# Define _mapping sin parámetros de llamada.
def _mapping():
    # Establece slot_stride a partir de 128.
    slot_stride = 128
    # Obtiene memory con bytearray(GLOBAL_HEADER_SIZE + 3 * slot_stride).
    memory = bytearray(GLOBAL_HEADER_SIZE + 3 * slot_stride)
    # Establece memory[0:4] a partir de b'ORVX'.
    memory[0:4] = b"ORVX"
    # Ejecuta struct.pack_into("<H", memory, 4, 1).
    struct.pack_into("<H", memory, 4, 1)
    # Ejecuta struct.pack_into("<H", memory, 6, GLOBAL_HEADER_SIZE).
    struct.pack_into("<H", memory, 6, GLOBAL_HEADER_SIZE)
    # Ejecuta struct.pack_into("<Q", memory, 8, len(memory)).
    struct.pack_into("<Q", memory, 8, len(memory))
    # Ejecuta struct.pack_into("<I", memory, 16, 3).
    struct.pack_into("<I", memory, 16, 3)
    # Ejecuta struct.pack_into("<I", memory, 20, SLOT_HEADER_SIZE).
    struct.pack_into("<I", memory, 20, SLOT_HEADER_SIZE)
    # Ejecuta struct.pack_into("<Q", memory, 24, 64).
    struct.pack_into("<Q", memory, 24, 64)
    # Ejecuta struct.pack_into("<Q", memory, 32, slot_stride).
    struct.pack_into("<Q", memory, 32, slot_stride)
    # Ejecuta struct.pack_into("<I", memory, 40, 4).
    struct.pack_into("<I", memory, 40, 4)
    # Ejecuta struct.pack_into("<I", memory, 44, 2).
    struct.pack_into("<I", memory, 44, 2)
    # Ejecuta struct.pack_into("<I", memory, 48, 4).
    struct.pack_into("<I", memory, 48, 4)
    # Ejecuta struct.pack_into("<I", memory, 52, 1).
    struct.pack_into("<I", memory, 52, 1)
    # Ejecuta struct.pack_into("<I", memory, 56, 30).
    struct.pack_into("<I", memory, 56, 30)
    # Ejecuta struct.pack_into("<I", memory, 60, 1).
    struct.pack_into("<I", memory, 60, 1)
    # Ejecuta struct.pack_into("<I", memory, 80, 1).
    struct.pack_into("<I", memory, 80, 1)
    # Ejecuta struct.pack_into("<I", memory, 84, 0).
    struct.pack_into("<I", memory, 84, 0)
    # Ejecuta struct.pack_into("<Q", memory, 88, 1).
    struct.pack_into("<Q", memory, 88, 1)

    # Establece base a partir de GLOBAL_HEADER_SIZE.
    base = GLOBAL_HEADER_SIZE
    # Obtiene payload (carga útil del fotograma) con bytes(range(12)).
    payload = bytes(range(12))
    # Ejecuta struct.pack_into("<Q", memory, base, 2).
    struct.pack_into("<Q", memory, base, 2)
    # Ejecuta struct.pack_into("<Q", memory, base + 8, 1).
    struct.pack_into("<Q", memory, base + 8, 1)
    # Ejecuta struct.pack_into("<q", memory, base + 16, 333_333).
    struct.pack_into("<q", memory, base + 16, 333_333)
    # Ejecuta struct.pack_into("<Q", memory, base + 24, len(payload)).
    struct.pack_into("<Q", memory, base + 24, len(payload))
    # Ejecuta struct.pack_into("<I", memory, base + 32, 4).
    struct.pack_into("<I", memory, base + 32, 4)
    # Ejecuta struct.pack_into("<I", memory, base + 36, 2).
    struct.pack_into("<I", memory, base + 36, 2)
    # Ejecuta struct.pack_into("<I", memory, base + 40, 4).
    struct.pack_into("<I", memory, base + 40, 4)
    # Ejecuta struct.pack_into("<I", memory, base + 44, 1).
    struct.pack_into("<I", memory, base + 44, 1)
    # Ejecuta struct.pack_into("<I", memory, base + 48, 2).
    struct.pack_into("<I", memory, base + 48, 2)
    # Establece memory[base + SLOT_HEADER_SIZE:base + SLOT_HEADER_SIZE + 12] a partir de payload.
    memory[base + SLOT_HEADER_SIZE : base + SLOT_HEADER_SIZE + 12] = payload
    # Devuelve memory.
    return memory


# Comprueba reads header and consistent frame en la suite de pruebas.
def test_reads_header_and_consistent_frame():
    # Obtiene memory con _mapping().
    memory = _mapping()
    # Obtiene header con read_header(memoryview(memory)).
    header = read_header(memoryview(memory))
    # Guarda en frame (fotograma) el resultado de read_latest_frame, que lee una copia estable del fotograma publicado.
    frame = read_latest_frame(memoryview(memory))

    # Verifica que se cumpla header.pixel_format == 'NV12'.
    assert header.pixel_format == "NV12"
    # Verifica que se cumpla header.slot_count == 3.
    assert header.slot_count == 3
    # Verifica que se cumpla frame is not None.
    assert frame is not None
    # Verifica que se cumpla frame.sequence == 1.
    assert frame.sequence == 1
    # Verifica que se cumpla frame.payload == bytes(range(12)).
    assert frame.payload == bytes(range(12))

    # Ejecuta acknowledge_frame(memoryview(memory), frame.sequence).
    acknowledge_frame(memoryview(memory), frame.sequence)
    # Verifica que se cumpla struct.unpack_from('<Q', memory, 96)[0] == 1.
    assert struct.unpack_from("<Q", memory, 96)[0] == 1


# Comprueba rejects slot while producer is writing en la suite de pruebas.
def test_rejects_slot_while_producer_is_writing():
    # Obtiene memory con _mapping().
    memory = _mapping()
    # Ejecuta struct.pack_into("<Q", memory, GLOBAL_HEADER_SIZE, 3).
    struct.pack_into("<Q", memory, GLOBAL_HEADER_SIZE, 3)
    # Verifica que se cumpla read_latest_frame(memoryview(memory), retries=2) is None.
    assert read_latest_frame(memoryview(memory), retries=2) is None
