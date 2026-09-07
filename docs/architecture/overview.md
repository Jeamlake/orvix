# ORVIX Architecture

## Logical Architecture

```text
Physical Camera
      |
      v
Operating-system camera driver
      |
      v
Native capture backend
      |
      v
ORVIX Capture
C++20
      |
      v
ORVIX Bridge
Shared Memory
      |
      v
ORVIX Vision
Python / NumPy / OpenCV
      |
      +------------------+
      |                  |
      v                  v
Processed Video     ORVIX Insight
                    Metrics / SQLite
```

## Data Plane

Large frame payloads move through shared memory.

## Control Plane

Small future commands may include START, STOP, SET_RESOLUTION, SET_FPS and GET_STATUS. The exact mechanism is intentionally deferred.

## Supported Platforms

The common capture contract is implemented by one native backend per operating
system:

```text
Camera interface
      |
      +-- Windows -> Media Foundation
      +-- Linux   -> V4L2
      +-- macOS   -> AVFoundation
```

CMake compiles exactly one backend. Platform headers and libraries remain out
of the common CLI, sequence, metadata, FPS and logging modules.

## Capture Backends

- Windows uses `IMFMediaSource` and `IMFSourceReader`.
- Linux uses V4L2 streaming I/O with kernel memory-mapped buffers.
- macOS uses `AVCaptureSession` and `AVCaptureVideoDataOutput` callbacks.

Each native capture flow owns its operating-system resources, keeps the
configured media type and assigns a monotonically increasing ORVIX sequence
number to every accepted sample. Native timestamps are normalized to
100-nanosecond units and provide the elapsed media time used to calculate
effective capture FPS.

## Native Observability

```text
Native backend result / stream state
                 |
                 v
Stable ORV-CAP diagnostic
                 |
          +------+------+
          |             |
          v             v
     CLI stderr    Structured log
                   logs/orvix-capture.log
```

The CLI records camera lifecycle, negotiated format, capture completion and
controlled failures. Frames remain ephemeral; the log contains diagnostics and
aggregate capture measurements rather than image payloads.
