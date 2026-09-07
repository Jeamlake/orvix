# Cross-Platform Native Validation

Status: **CI validated; Windows hardware validated**

Issue: **#15**

## Backend selection

| Runner | Compiled backend | Result |
|---|---|---|
| `windows-latest` | Windows Media Foundation | PASS |
| `ubuntu-latest` | Linux V4L2 | PASS |
| `macos-latest` | macOS AVFoundation | PASS |

The native jobs configure from a clean checkout, build the platform-selected
sources and run CTest. `TC-CAP-003-PLATFORM` verifies that each factory creates
the correct enumerator and camera without requiring hardware.

Python 3.11 package installation and pytest pass independently on Windows,
Ubuntu and macOS. Python 3.8 also passes as the legacy Windows 7 language path.

## CI evidence

- status: completed
- conclusion: success
- commit: 58f1fa743c0f1715bf4166e0f0e5bee0cbf3df1b
- run: https://github.com/Jeamlake/orvix/actions/runs/34118735938

## Physical Windows regression

The refactored platform-neutral CLI retained the existing physical result:

```text
Backend: Windows Media Foundation
Device: GENERAL WEBCAM
Negotiated resolution: 1280x720
Negotiated pixel format: NV12
Negotiated FPS: 30.00
Frames captured: 120
First sequence: 1
Last sequence: 120
Sequence status: STRICTLY_INCREASING
Timestamp status: STRICTLY_INCREASING
Measured capture FPS: 19.47
Capture status: CAPTURE_COMPLETED
```

## Hardware boundary

GitHub-hosted Ubuntu and macOS runners do not expose webcams. Their native
backends are compiler-, linker- and hardware-independent test validated. A
physical capture on Linux, macOS and Windows 7 requires access to those hosts
and is documented as an explicit release check rather than represented as a CI
hardware result.
