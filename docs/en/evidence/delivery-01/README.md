# Delivery 1 — Foundation and Native Capture

Status: **Functional scope complete; Windows hardware validated**

This directory collects the reproducible evidence for the first graded ORVIX
delivery. Native camera behavior is validated on the Windows development
workstation; hardware-independent logic and platform backends are exercised by
CTest and GitHub Actions on Windows, Linux and macOS.

## Evidence index

| Scope | Requirement or criterion | Result | Evidence |
|---|---|---|---|
| Toolchain | CA-008 | PASS | [Environment validation](environment-validation.md) |
| Device enumeration | RF-001 / CA-001 | PASS | [RF-001](rf-001-device-enumeration.md) |
| Device selection | RF-002 | PASS | [RF-002](rf-002-camera-selection.md) |
| Camera opening | RF-003 / CA-002 | PASS | [RF-003](rf-003-open-selected-camera.md) |
| Format negotiation | RNF-001 | PASS | [Format negotiation](format-negotiation.md) |
| Continuous capture | RF-004 / CA-003 / CA-004 | PASS | [RF-004](rf-004-continuous-frame-capture.md) |
| Frame metadata | RF-005 / CA-005 | PASS | [RF-005](rf-005-frame-metadata.md) |
| Capture FPS | RF-015 / CA-006 | PASS | [RF-015](rf-015-capture-fps.md) |
| Camera diagnostics | RF-023 / CA-007 | PASS | [RF-023](rf-023-camera-failure-handling.md) |
| Structured logging | RF-026 | PASS | [RF-026](rf-026-structured-logging.md) |
| Automated validation | CA-009 | PASS | [GitHub Actions validation](#github-actions-validation) |

## Reproduction

From Git Bash in the repository root on Windows:

```bash
./scripts/setup.sh
./scripts/build.sh
./scripts/test.sh
./build/native/Release/orvix-capture.exe devices
./build/native/Release/orvix-capture.exe capture --index 0
```

On Linux and macOS, the executable path is `./build/native/orvix-capture`.

The last command activates the physical webcam. Its expected proof is 120
per-frame records, `Frames captured: 120`, `First sequence: 1`, `Last sequence:
120`, `Sequence status: STRICTLY_INCREASING` and `Capture status:
CAPTURE_COMPLETED`.

## GitHub Actions validation

- status: completed
- conclusion: success
- commit: 4e37418f655d65d5dbbb444d615c31e929329982
- run: https://github.com/Jeamlake/orvix/actions/runs/34170322943
- Native C++ on Windows, Linux and macOS: **PASS**
- Python 3.11 on Windows, Linux and macOS: **PASS**
- Python 3.8 compatibility: **PASS**

The CI jobs perform clean native builds and run all 14 CTest entries. Camera
hardware is not required by CI; physical Windows capture is recorded in the
requirement evidence pages.

## Delivery boundary

Delivery 1 ends at native capture, per-frame metadata, capture FPS and
diagnostics. Shared-memory transport and Python consumption begin in Delivery
2. Frames remain ephemeral and no camera image is saved by default.

The later native portability extension is recorded in
[`docs/en/evidence/cross-platform-native-validation.md`](../cross-platform-native-validation.md).
