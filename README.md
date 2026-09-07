# ORVIX

**Optical Real-time Video Ingestion eXchange**

ORVIX is a modular real-time video ingestion and computer vision platform developed for the course **Programación de Interfaces y Dispositivos Periféricos**.

## Problem

High-level prototypes commonly open a webcam directly through APIs such as `cv2.VideoCapture(0)`. That approach is useful for prototypes but hides most of the I/O path between the physical device, operating system, native multimedia API, frame buffers and high-level application.

ORVIX separates those responsibilities.

## Architecture

```text
Physical Camera
      |
      v
Windows Driver
      |
      v
Media Foundation
      |
      v
ORVIX Capture (C++20)
      |
      v
ORVIX Bridge (Shared Memory)
      |
      v
ORVIX Vision (Python / NumPy / OpenCV)
      |
      v
ORVIX Insight (Telemetry)
```

## Components

### ORVIX Capture
Native C++ subsystem responsible for device enumeration, camera selection, format negotiation, frame acquisition and capture telemetry.

### ORVIX Bridge
Inter-process communication subsystem responsible for shared-memory frame transport.

### ORVIX Vision
Python subsystem responsible for high-level frame processing.

### ORVIX Insight
Observability subsystem responsible for FPS, dropped frames, latency, detections and historical metrics.

## Delivery roadmap

### Delivery 1 — Foundation + Native Capture
Repository, architecture, requirements, C++20, Media Foundation, camera enumeration, selection, format negotiation, frame acquisition, metadata, basic FPS, logging, tests and CI.

### Delivery 2 — ORVIX Bridge
IPC Protocol v1, shared memory, ring buffer, synchronization, Python consumer, NumPy and OpenCV visualization.

### Delivery 3 — Vision + Observability
Filters, detection pipeline, FPS analysis, dropped frames, latency, detections/minute and SQLite telemetry.

### Delivery 4 — ORVIX 1.0
Dashboard, configuration, recovery, automated testing, benchmarks, reproducibility, documentation and academic release.

## Technology stack

- C++20
- MSVC
- Windows SDK
- Windows Media Foundation
- CMake
- Ninja
- Python 3.11
- NumPy
- OpenCV
- SQLite
- pytest
- GitHub Actions

## Platform

Windows 10/11 x64.

## Native capture CLI

```text
orvix-capture devices
orvix-capture select --index <N>
orvix-capture open --index <N>
orvix-capture formats --index <N>
orvix-capture capture --index <N>
```

The `capture` command opens the selected camera, negotiates the closest native
format to the initial 1280x720 at 30 FPS target and reads 120 consecutive video
samples. It reports sequence, Media Foundation timestamp, resolution, pixel
format and byte count for every frame, followed by the measured capture FPS.

Every command writes structured lifecycle and failure diagnostics to
`logs/orvix-capture.log`. Expected failures include a stable diagnostic code
and a non-zero process exit code.

Delivery 1 validation records are indexed in
[`docs/evidence/delivery-01/README.md`](docs/evidence/delivery-01/README.md).

## Privacy

Frames are ephemeral by default and are not persisted automatically.

## Current milestone

**Delivery 1 — Foundation + Native Capture (functional scope complete)**

## License

MIT
