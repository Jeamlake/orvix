# ORVIX IPC Protocol v1

[English](ipc-v1.md) | [Español](../../es/protocol/ipc-v1.md)

Status: **Implemented**

Delivery: **2**

## Purpose

The C++ process exclusively owns the camera and publishes every frame through
shared memory. Python attaches by name, copies the latest stable slot, interprets
the format with NumPy and displays it with OpenCV. No temporary files are used,
and Python never opens the camera.

## Layout

```text
Global header: 128 bytes
Slot 0: 64-byte header + payload
Slot 1: 64-byte header + payload
Slot 2: 64-byte header + payload
```

The default name is `orvix-camera-v1`. Integers use little-endian encoding and
all multibyte offsets are naturally aligned.

## Global header

| Offset | Size | Type | Field |
|---:|---:|---|---|
| 0 | 4 | bytes | `ORVX` magic |
| 4 | 2 | uint16 | Version, value `1` |
| 6 | 2 | uint16 | Header size, `128` |
| 8 | 8 | uint64 | Total mapping size |
| 16 | 4 | uint32 | Slot count |
| 20 | 4 | uint32 | Slot header size, `64` |
| 24 | 8 | uint64 | Per-slot payload capacity |
| 32 | 8 | uint64 | Slot stride |
| 40 | 4 | uint32 | Configured width |
| 44 | 4 | uint32 | Configured height |
| 48 | 4 | uint32 | Frame stride |
| 52 | 4 | uint32 | Pixel-format code |
| 56 | 4 | uint32 | FPS numerator |
| 60 | 4 | uint32 | FPS denominator |
| 64 | 8 | int64 | Creation time in 100 ns units |
| 72 | 8 | uint64 | Producer PID |
| 80 | 4 | uint32 | Producer state |
| 84 | 4 | uint32 | Latest published slot |
| 88 | 8 | uint64 | Latest published sequence |
| 96 | 8 | uint64 | Latest consumed sequence |
| 104 | 8 | uint64 | Overwritten frames |
| 112 | 8 | uint64 | Producer FPS multiplied by 1000 |
| 120 | 8 | bytes | Reserved |

Producer states are `0 INITIALIZING`, `1 RUNNING`, `2 STOPPED` and `3 FAILED`.

## Per-slot header

| Offset | Size | Type | Field |
|---:|---:|---|---|
| 0 | 8 | uint64 | Seqlock generation |
| 8 | 8 | uint64 | Sequence |
| 16 | 8 | int64 | Timestamp in 100 ns units |
| 24 | 8 | uint64 | Valid payload bytes |
| 32 | 4 | uint32 | Width |
| 36 | 4 | uint32 | Height |
| 40 | 4 | uint32 | Stride |
| 44 | 4 | uint32 | Pixel-format code |
| 48 | 4 | uint32 | Slot state |
| 52 | 12 | bytes | Reserved |
| 64 | variable | bytes | Frame payload |

Slot states are `0 EMPTY`, `1 WRITING` and `2 READY`. Pixel-format codes are
`NV12=1`, `YUY2=2`, `UYVY=3`, `I420=4`, `YV12=5`, `RGB24=6`, `BGRA32=7`,
`ARGB32=8`, `MJPG=9` and `H264=10`.

## Synchronization

The producer makes the slot generation odd, writes metadata and payload, marks
the slot ready and publishes an even generation. It then updates the global
slot index and sequence. The consumer reads the generation before and after its
copy and accepts the frame only when both values match and are even. It then
writes the consumed sequence at offset 96.

The producer never waits for Python. If the consumer falls behind, the ring
keeps the three newest frames and counts overwritten frames. This policy keeps
latency low and lets Capture continue when Vision exits or fails.

## Lifecycle

The producer exclusively creates the mapping, publishes `RUNNING`, emits frames
and finishes with `STOPPED` or `FAILED`. Windows uses `CreateFileMapping`; Linux
and macOS use `shm_open` and `mmap`. The mapping is released when the producer
exits; an already attached consumer can finish reading its active view.
