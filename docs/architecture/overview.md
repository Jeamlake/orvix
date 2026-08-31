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