# Delivery 2 — ORVIX Bridge and RAW Visualization

[English](README.md) | [Español](../../../es/evidence/delivery-02/README.md)

Status: **Functional scope complete; physical webcam validated on Windows**

Delivery 2 connects Capture and Vision through a versioned binary protocol.
The C++ process retains exclusive camera ownership; Python only reads shared
memory, creates NumPy arrays and presents video through OpenCV.

## Verified scope

| Requirement | Result | Evidence |
|---|---|---|
| RF-006 — Shared-memory publication | PASS | `ORVX` header, protocol v1 and three-slot ring |
| RF-007 — Python consumption | PASS | C++ to Python cross-process integration test |
| RF-008 — NumPy interpretation | PASS | NV12 decoding and BGR geometry test |
| RF-009 / RF-010 — Window and RAW mode | PASS | `ORVIX Live - RAW` viewer and presentation-flow test |
| RF-017 — Skipped frames | PASS | `SequenceTracker` detects gaps and reports them on screen |
| RF-020 — Buffer utilization | PASS | Published/consumed sequences, occupancy and overwrites |
| RF-024 — Process isolation | PASS | Producer completes when consumer exits early |
| RF-025 — Synthetic source | PASS | Reproducible 1280x720, 30 FPS NV12 video |

The binary contract is defined in [IPC protocol v1](../../protocol/ipc-v1.md).

## Local validation

The complete path was exercised on September 8, 2026 with the `GENERAL WEBCAM`
device and the Windows Media Foundation backend:

| Measurement | Result |
|---|---|
| Negotiated format | 1280x720, NV12, nominal 30 FPS |
| Frames received by Python | 30 |
| First/last sequence observed | 1 / 30 |
| Frames skipped while consuming | 0 |
| Frames completed by C++ | 180 |
| Measured physical FPS | 19.46 |
| Final producer state | `COMPLETED` |

Python exited after 30 frames and C++ continued to 180. The final 147
overwrites are frames produced after the consumer left and demonstrate the
ring's nonblocking policy.

Local automated validation produced:

- Native CTest: **16/16 PASS**.
- Python unit tests: **6/6 PASS**.
- Synthetic C++ → shared memory → Python integration: **PASS**.

## Reproduction

The [demonstration guide](demonstration-guide.md) provides Git Bash,
PowerShell, Linux and macOS commands and describes the expected terminal output
and video window.

Frames remain in memory and no image or video is saved automatically.
