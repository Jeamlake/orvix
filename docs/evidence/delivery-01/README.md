# Delivery 1 — Foundation and Native Capture

Status: **Functionally complete on the development workstation**

This directory collects the reproducible evidence for the first graded ORVIX
delivery. Native camera behavior is validated on the development workstation;
hardware-independent logic is also exercised by CTest and GitHub Actions.

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

From a Visual Studio Developer PowerShell in the repository root:

```powershell
cmake -S . -B out/delivery-01-validation -DORVIX_BUILD_TESTS=ON
cmake --build out/delivery-01-validation --config Release --parallel
ctest --test-dir out/delivery-01-validation -C Release --output-on-failure
./out/delivery-01-validation/native/Release/orvix-capture.exe devices
./out/delivery-01-validation/native/Release/orvix-capture.exe capture --index 0
```

The last command activates the physical webcam. Its expected proof is 120
per-frame records, `Frames captured: 120`, `First sequence: 1`, `Last sequence:
120`, `Sequence status: STRICTLY_INCREASING` and `Capture status:
CAPTURE_COMPLETED`.

## GitHub Actions validation

- status: completed
- conclusion: success
- commit: d42a7adfe18c74d8daf76b29f553e83d06913c60
- run: https://github.com/Jeamlake/orvix/actions/runs/34084335173
- Native C++ Validation: **PASS**
- Python Validation: **PASS**

The native job performs a clean MSVC build and runs all 13 CTest entries. The
Python job installs the package on Python 3.11 and runs pytest. Camera hardware
is not required by either CI job; the physical Media Foundation capture is
recorded in the requirement evidence pages.

## Delivery boundary

Delivery 1 ends at native Media Foundation capture, per-frame metadata, capture
FPS and diagnostics. Shared-memory transport and Python consumption begin in
Delivery 2. Frames remain ephemeral and no camera image is saved by default.
