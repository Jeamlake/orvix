# ORVIX Software Requirements Specification

Version: 0.1

## Functional Requirements

| ID | Requirement | Priority | Delivery |
|---|---|---|---|
| RF-001 | Enumerate available video capture devices. | Critical | 1 |
| RF-002 | Allow selection of a capture device. | Critical | 1 |
| RF-003 | Open the selected device using Media Foundation from C++. | Critical | 1 |
| RF-004 | Continuously acquire frames. | Critical | 1 |
| RF-005 | Expose frame metadata. | Critical | 1 |
| RF-006 | Publish frames through shared memory. | Critical | 2 |
| RF-007 | Allow Python to consume shared frames without opening the camera directly. | Critical | 2 |
| RF-008 | Interpret frame bytes using NumPy. | Critical | 2 |
| RF-009 | Display received video. | Critical | 2 |
| RF-010 | Provide RAW visualization. | High | 2 |
| RF-011 | Provide grayscale processing. | High | 3 |
| RF-012 | Provide edge detection. | High | 3 |
| RF-013 | Provide an OpenCV detection algorithm. | Critical | 3 |
| RF-014 | Change processing modes without restarting capture. | High | 3 |
| RF-015 | Measure capture FPS. | Critical | 1 |
| RF-016 | Measure processing FPS. | Critical | 3 |
| RF-017 | Detect skipped frames. | Critical | 2 |
| RF-018 | Estimate processing latency. | High | 3 |
| RF-019 | Count detections per minute. | Critical | 3 |
| RF-020 | Report shared-memory utilization. | Critical | 2 |
| RF-021 | Persist historical telemetry. | High | 3 |
| RF-022 | Expose session information. | High | 3 |
| RF-023 | Detect camera disconnection. | High | 1 |
| RF-024 | Vision failure must not necessarily terminate capture. | High | 2 |
| RF-025 | Provide a synthetic frame source. | High | 2 |
| RF-026 | Log relevant events and failures. | High | 1 |

## Architectural Requirement

### RF-ARCH-001

Python shall not directly access the camera during the normal ORVIX execution path.

```text
Camera
-> Driver
-> Media Foundation
-> C++
-> Shared Memory
-> Python
-> NumPy
-> OpenCV
```

## Non-functional Requirements

- RNF-001 Performance: initial target 1280x720 @ 30 FPS.
- RNF-002 Low latency: avoid temporary files and unnecessary serialization.
- RNF-003 Minimize copies between shared memory and NumPy where practical.
- RNF-004 Modularity between Capture, Bridge, Vision and Insight.
- RNF-005 Structured observability.
- RNF-006 Reproducible clean checkout.
- RNF-007 Maintainable module boundaries.
- RNF-008 Hardware-independent automated tests.
- RNF-009 CI on pushes and pull requests.
- RNF-010 Frames are not persisted by default.
- RNF-011 Controlled diagnostics for expected failures.
- RNF-012 Explicit IPC protocol versioning.

## Delivery 1 Acceptance Criteria

| ID | Criterion |
|---|---|
| CA-001 | At least one webcam can be enumerated. |
| CA-002 | C++ opens the webcam. |
| CA-003 | Consecutive frames can be acquired. |
| CA-004 | Frame sequence increases. |
| CA-005 | Frame metadata can be reported. |
| CA-006 | Capture FPS can be measured. |
| CA-007 | Expected camera failures produce diagnostics. |
| CA-008 | Clean checkout can be built using documentation. |
| CA-009 | Hardware-independent CI succeeds. |