# ORVIX Architecture

## Logical Architecture

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

## Initial Platform

Windows x64.

## Capture Backend

Windows Media Foundation.

The native capture flow owns the `IMFMediaSource` and `IMFSourceReader`, keeps
the configured media type, and assigns a monotonically increasing ORVIX
sequence number to each accepted sample. Media Foundation presentation
timestamps remain in their native 100-nanosecond units and provide the elapsed
media time used to calculate effective capture FPS.

## Native Observability

```text
Media Foundation result / stream flag
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
