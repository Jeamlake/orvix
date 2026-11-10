# ORVIX

## Desktop application · ORVIX 1.0

ORVIX captures video in C++, publishes frames through shared memory and
switches between RAW, grayscale, edges and motion detection live in Python/Qt.
The desktop app includes live metrics, SQLite history, session charts,
settings and diagnostics.

In the Windows x64 distribution, extract the entire
`Aplicacion-Windows-x64` folder and run `ORVIX.exe`. Start with the synthetic
demo. See the [ORVIX 1.0 guide](docs/en/release-1.0.md) for source builds,
reproducible benchmarks and packaging.

The [Avance 2 report, slides and source ZIP](https://github.com/Jeamlake/orvix/tree/8e88e67e717c72a161c509cb90a50161f63f7f48/entregables/Avance2_ORVIX_2026-10-11) and
[their source documents and mockups](https://github.com/Jeamlake/orvix/tree/8e88e67e717c72a161c509cb90a50161f63f7f48/docs/es/academic/avance-02) remain available in the
academic snapshot. The active engine is in [src/native/](src/native/) and the desktop
application is in [src/python/src/orvix/](src/python/src/orvix/).

[English](README.md) | [Español](README.es.md)

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
Operating-system camera driver
      |
      v
Native capture backend
Media Foundation / V4L2 / AVFoundation
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
Native C++ subsystem responsible for device enumeration, camera selection,
format negotiation, frame acquisition and capture telemetry. CMake selects
Media Foundation on Windows, V4L2 on Linux and AVFoundation on macOS.

### ORVIX Bridge
Inter-process communication subsystem responsible for shared-memory frame transport.

### ORVIX Vision
Python subsystem responsible for high-level frame processing.

### ORVIX Insight
Observability subsystem responsible for FPS, dropped frames, latency, detections and historical metrics.

## Delivery roadmap

### Delivery 1 — Foundation + Native Capture
Repository, architecture, requirements, C++20, native Windows/Linux/macOS camera
backends, camera enumeration, selection, format negotiation, frame acquisition,
metadata, basic FPS, logging, tests and cross-platform CI.

### Delivery 2 — ORVIX Bridge
IPC Protocol v1, shared memory, ring buffer, synchronization, Python consumer, NumPy and OpenCV visualization.

### Delivery 3 — Vision + Observability
Filters, detection pipeline, FPS analysis, dropped frames, latency, detections/minute and SQLite telemetry.

### Delivery 4 — ORVIX 1.0
Dashboard, configuration, recovery, automated testing, benchmarks, reproducibility, documentation and academic release.

## Technology stack

- C++20
- MSVC, GCC and Apple Clang
- Windows Media Foundation
- Linux Video4Linux2 (V4L2)
- macOS AVFoundation
- CMake
- Ninja
- Python 3.8–3.13
- NumPy
- OpenCV
- SQLite
- pytest
- GitHub Actions

## Platforms

| Operating system | Native backend | Status |
|---|---|---|
| Windows 10/11 | Media Foundation | CI and physical camera validated |
| Windows 7 SP1 | Media Foundation legacy build | Source and Python 3.8 compatibility path |
| Linux | V4L2 | CI validated |
| macOS | AVFoundation | CI validated |

Build prerequisites and platform-specific camera permissions are documented in
[`docs/en/platform/compatibility.md`](docs/en/platform/compatibility.md).

## Native capture CLI

```text
orvix-capture devices
orvix-capture select --index <N>
orvix-capture open --index <N>
orvix-capture formats --index <N>
orvix-capture capture --index <N>
orvix-capture bridge --index <N> --frames 900
orvix-capture bridge --synthetic --frames 900
```

The `capture` command opens the selected camera, negotiates the closest native
format to the initial 1280x720 at 30 FPS target and reads 120 consecutive video
samples. It reports sequence, native sample timestamp, resolution, pixel format
and byte count for every frame, followed by the measured capture FPS.

Every command writes structured lifecycle and failure diagnostics to
`logs/orvix-capture.log`. Expected failures include a stable diagnostic code
and a non-zero process exit code.

## Bridge and RAW viewer

With the `bridge` producer running, open a second terminal and execute:

```text
python -m orvix.ui.viewer
```

Python receives frames through shared memory, interprets them with NumPy and
opens the **ORVIX Live - RAW** window through OpenCV. `Q` or `Esc` closes the
viewer without interrupting the C++ producer. Headless validation is available
through `python -m orvix.ui.viewer --headless --frames 30`.

Delivery 1 validation records are indexed in
[`docs/en/evidence/delivery-01/README.md`](docs/en/evidence/delivery-01/README.md).
Delivery 2 evidence and presentation instructions are indexed in
[`docs/en/evidence/delivery-02/README.md`](docs/en/evidence/delivery-02/README.md).

The complete documentation is available in [English](docs/en/README.md) and
[Spanish](docs/es/README.md).

## Privacy

Frames are ephemeral by default and are not persisted automatically.

## Current milestone

**Version 1.0.0 · Capture, Bridge, Vision and Insight**

## License

MIT
