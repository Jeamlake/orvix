"""Versioned ORVIX shared-memory wire protocol."""

from __future__ import annotations

import struct
from dataclasses import dataclass
from typing import Optional

MAGIC = b"ORVX"
PROTOCOL_VERSION = 1
GLOBAL_HEADER_SIZE = 128
SLOT_HEADER_SIZE = 64
DEFAULT_SHARED_MEMORY_NAME = "orvix-camera-v1"

PRODUCER_INITIALIZING = 0
PRODUCER_RUNNING = 1
PRODUCER_STOPPED = 2
PRODUCER_FAILED = 3

PIXEL_FORMATS = {
    0: "UNKNOWN",
    1: "NV12",
    2: "YUY2",
    3: "UYVY",
    4: "I420",
    5: "YV12",
    6: "RGB24",
    7: "BGRA32",
    8: "ARGB32",
    9: "MJPG",
    10: "H264",
}


class ProtocolError(RuntimeError):
    """Raised when a mapping does not satisfy the ORVIX IPC contract."""


@dataclass(frozen=True)
class SharedHeader:
    total_size: int
    slot_count: int
    slot_header_size: int
    payload_capacity: int
    slot_stride: int
    width: int
    height: int
    frame_stride: int
    pixel_format: str
    fps_numerator: int
    fps_denominator: int
    created_timestamp_100ns: int
    producer_pid: int
    producer_state: int
    published_slot: int
    published_sequence: int
    consumed_sequence: int
    overwritten_frames: int
    producer_fps: float

    @property
    def buffer_utilization(self) -> float:
        pending = max(0, self.published_sequence - self.consumed_sequence)
        return min(pending, self.slot_count) / self.slot_count


@dataclass(frozen=True)
class SharedFrame:
    sequence: int
    timestamp_100ns: int
    width: int
    height: int
    stride: int
    pixel_format: str
    payload: bytes


def _u16(buffer: memoryview, offset: int) -> int:
    return struct.unpack_from("<H", buffer, offset)[0]


def _u32(buffer: memoryview, offset: int) -> int:
    return struct.unpack_from("<I", buffer, offset)[0]


def _u64(buffer: memoryview, offset: int) -> int:
    return struct.unpack_from("<Q", buffer, offset)[0]


def _i64(buffer: memoryview, offset: int) -> int:
    return struct.unpack_from("<q", buffer, offset)[0]


def read_header(buffer: memoryview) -> SharedHeader:
    """Validate and decode the fixed 128-byte global header."""

    if len(buffer) < GLOBAL_HEADER_SIZE:
        raise ProtocolError("Shared memory is smaller than the ORVIX header.")
    if bytes(buffer[0:4]) != MAGIC:
        raise ProtocolError("Shared memory does not contain ORVIX magic bytes.")
    version = _u16(buffer, 4)
    if version != PROTOCOL_VERSION:
        raise ProtocolError(
            "Unsupported ORVIX protocol version: {}.".format(version)
        )
    header_size = _u16(buffer, 6)
    if header_size != GLOBAL_HEADER_SIZE:
        raise ProtocolError("Invalid ORVIX global-header size.")

    total_size = _u64(buffer, 8)
    slot_count = _u32(buffer, 16)
    slot_header_size = _u32(buffer, 20)
    payload_capacity = _u64(buffer, 24)
    slot_stride = _u64(buffer, 32)
    if slot_count < 2 or slot_header_size != SLOT_HEADER_SIZE:
        raise ProtocolError("Invalid ORVIX ring-buffer geometry.")
    expected = GLOBAL_HEADER_SIZE + slot_count * slot_stride
    if total_size != expected or total_size > len(buffer):
        raise ProtocolError("ORVIX mapping size does not match its header.")

    format_code = _u32(buffer, 52)
    return SharedHeader(
        total_size=total_size,
        slot_count=slot_count,
        slot_header_size=slot_header_size,
        payload_capacity=payload_capacity,
        slot_stride=slot_stride,
        width=_u32(buffer, 40),
        height=_u32(buffer, 44),
        frame_stride=_u32(buffer, 48),
        pixel_format=PIXEL_FORMATS.get(format_code, "UNKNOWN"),
        fps_numerator=_u32(buffer, 56),
        fps_denominator=_u32(buffer, 60),
        created_timestamp_100ns=_i64(buffer, 64),
        producer_pid=_u64(buffer, 72),
        producer_state=_u32(buffer, 80),
        published_slot=_u32(buffer, 84),
        published_sequence=_u64(buffer, 88),
        consumed_sequence=_u64(buffer, 96),
        overwritten_frames=_u64(buffer, 104),
        producer_fps=_u64(buffer, 112) / 1000.0,
    )


def read_latest_frame(
    buffer: memoryview,
    retries: int = 20,
) -> Optional[SharedFrame]:
    """Copy the newest complete frame using the slot seqlock."""

    if retries < 1:
        raise ValueError("retries must be positive")
    for _ in range(retries):
        header = read_header(buffer)
        if header.published_sequence == 0:
            return None
        if header.published_slot >= header.slot_count:
            raise ProtocolError("Published slot is outside the ORVIX ring.")

        base = GLOBAL_HEADER_SIZE + header.published_slot * header.slot_stride
        generation_before = _u64(buffer, base)
        if generation_before & 1:
            continue

        sequence = _u64(buffer, base + 8)
        timestamp = _i64(buffer, base + 16)
        payload_size = _u64(buffer, base + 24)
        width = _u32(buffer, base + 32)
        height = _u32(buffer, base + 36)
        stride = _u32(buffer, base + 40)
        format_code = _u32(buffer, base + 44)
        state = _u32(buffer, base + 48)
        if payload_size > header.payload_capacity:
            raise ProtocolError("Frame payload exceeds its declared capacity.")
        if state != 2:
            continue

        start = base + SLOT_HEADER_SIZE
        payload = bytes(buffer[start : start + payload_size])
        generation_after = _u64(buffer, base)
        if (
            generation_before == generation_after
            and not generation_after & 1
            and sequence == header.published_sequence
        ):
            return SharedFrame(
                sequence=sequence,
                timestamp_100ns=timestamp,
                width=width,
                height=height,
                stride=stride,
                pixel_format=PIXEL_FORMATS.get(format_code, "UNKNOWN"),
                payload=payload,
            )
    return None


def acknowledge_frame(buffer: memoryview, sequence: int) -> None:
    """Publish the last sequence copied by the Python consumer."""

    if sequence < 0:
        raise ValueError("sequence cannot be negative")
    struct.pack_into("<Q", buffer, 96, sequence)
