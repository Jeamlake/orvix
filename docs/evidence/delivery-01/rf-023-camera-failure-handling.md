# RF-023 — Camera Failure Handling

Status: **Validated**

Issue: **#12**

## Objective

Convert expected device and stream failures into stable diagnostics that a
person or supervising process can act on.

## Covered conditions

| Condition | Diagnostic | Exit |
|---|---|---:|
| No cameras enumerated for a camera command | ORV-CAP-100 | 65 |
| Index outside the device list | ORV-CAP-404 | 66 |
| Selected media source cannot be activated | ORV-CAP-501 | 71 |
| Format cannot be configured | ORV-CAP-502 | 72 |
| Device removed or invalidated during `ReadSample` | ORV-CAP-503 | 74 |
| Stream error, early end or prolonged lack of samples | ORV-CAP-503 | 74 |
| Active media type changes during capture | ORV-CAP-505 | 75 |

Each error is written to stderr and the structured log before the command
returns its documented non-zero exit code.

## Development-workstation validation

Invalid-index command:

```powershell
./out/rf005-rf026-final-validation/native/Release/orvix-capture.exe capture --index 999999
$LASTEXITCODE
```

Observed result:

```text
[ORV-CAP-404] Camera selection failed: Camera index 999999 is out of range. Available devices: 1
66
```

`TC-CAP-023` also simulates a camera that stops returning frames and verifies
the `ORV-CAP-503` controlled diagnostic after the configured empty-read limit.

The Media Foundation reader handles the platform's device-invalidated HRESULT,
the Windows device-not-connected HRESULT, source-reader error and end-of-stream
flags, and unexpected media-type changes. A physical hot-unplug was not forced
during this validation run; the hardware-independent stalled-stream test covers
the recovery boundary without requiring a camera in CI.

## Acceptance status

- absent-camera path: **PASS**
- invalid-index path on physical workstation: **PASS**
- camera opening and format failures mapped: **PASS**
- device invalidation and stream termination mapped: **PASS**
- stalled stream rejected by automated test: **PASS**
- stable diagnostic and exit codes: **PASS**
