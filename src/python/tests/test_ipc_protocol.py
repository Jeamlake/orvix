import struct

from orvix.ipc.protocol import (
    GLOBAL_HEADER_SIZE,
    SLOT_HEADER_SIZE,
    acknowledge_frame,
    read_header,
    read_latest_frame,
)


def _mapping():
    slot_stride = 128
    memory = bytearray(GLOBAL_HEADER_SIZE + 3 * slot_stride)
    memory[0:4] = b"ORVX"
    struct.pack_into("<H", memory, 4, 1)
    struct.pack_into("<H", memory, 6, GLOBAL_HEADER_SIZE)
    struct.pack_into("<Q", memory, 8, len(memory))
    struct.pack_into("<I", memory, 16, 3)
    struct.pack_into("<I", memory, 20, SLOT_HEADER_SIZE)
    struct.pack_into("<Q", memory, 24, 64)
    struct.pack_into("<Q", memory, 32, slot_stride)
    struct.pack_into("<I", memory, 40, 4)
    struct.pack_into("<I", memory, 44, 2)
    struct.pack_into("<I", memory, 48, 4)
    struct.pack_into("<I", memory, 52, 1)
    struct.pack_into("<I", memory, 56, 30)
    struct.pack_into("<I", memory, 60, 1)
    struct.pack_into("<I", memory, 80, 1)
    struct.pack_into("<I", memory, 84, 0)
    struct.pack_into("<Q", memory, 88, 1)

    base = GLOBAL_HEADER_SIZE
    payload = bytes(range(12))
    struct.pack_into("<Q", memory, base, 2)
    struct.pack_into("<Q", memory, base + 8, 1)
    struct.pack_into("<q", memory, base + 16, 333_333)
    struct.pack_into("<Q", memory, base + 24, len(payload))
    struct.pack_into("<I", memory, base + 32, 4)
    struct.pack_into("<I", memory, base + 36, 2)
    struct.pack_into("<I", memory, base + 40, 4)
    struct.pack_into("<I", memory, base + 44, 1)
    struct.pack_into("<I", memory, base + 48, 2)
    memory[base + SLOT_HEADER_SIZE : base + SLOT_HEADER_SIZE + 12] = payload
    return memory


def test_reads_header_and_consistent_frame():
    memory = _mapping()
    header = read_header(memoryview(memory))
    frame = read_latest_frame(memoryview(memory))

    assert header.pixel_format == "NV12"
    assert header.slot_count == 3
    assert frame is not None
    assert frame.sequence == 1
    assert frame.payload == bytes(range(12))

    acknowledge_frame(memoryview(memory), frame.sequence)
    assert struct.unpack_from("<Q", memory, 96)[0] == 1


def test_rejects_slot_while_producer_is_writing():
    memory = _mapping()
    struct.pack_into("<Q", memory, GLOBAL_HEADER_SIZE, 3)
    assert read_latest_frame(memoryview(memory), retries=2) is None
