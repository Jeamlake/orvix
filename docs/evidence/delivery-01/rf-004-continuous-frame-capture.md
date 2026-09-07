# RF-004 — Continuous Frame Acquisition

Status: **Validated**

Issue: **#8**

## Objective

Continuously acquire real video samples from the selected camera and prove that
the ORVIX frame sequence increases.

## Architecture

```text
Physical camera
      |
      v
IMFMediaSource
      |
      v
IMFSourceReader
      |
      v
ReadSample loop
      |
      v
Frame sequence + capture summary
```

## CLI

```text
orvix-capture capture --index <N>
```

The initial CLI capture session requests 120 frames and exits after completing
the bounded validation run.

## Development workstation validation

```text
Device: GENERAL WEBCAM
Backend: Windows Media Foundation
Target resolution: 1280x720
Target FPS: 30
Negotiated resolution: 1280x720
Negotiated pixel format: NV12
Negotiated FPS: 30.00
Negotiated native type index: 2

Frames requested: 120
Frames captured: 120
First sequence: 1
Last sequence: 120
Captured bytes: 165888000
Empty reads: 1
Sequence status: STRICTLY_INCREASING
Capture status: CAPTURE_COMPLETED
```

The captured byte count is consistent with 120 NV12 frames at 1280x720:
1,382,400 bytes per frame.

## Controlled failure handling

The capture path rejects:

- a zero-frame request;
- an invalid camera index;
- capture before camera opening;
- capture before format configuration;
- source-reader errors or premature end of stream;
- a media-type change during capture;
- excessive empty reads;
- byte-counter overflow.

## Automated validation

- synthetic reader tolerates an empty read: **PASS**
- requested sample count is reached: **PASS**
- sequence starts at 1 and ends at the requested count: **PASS**
- timestamps and byte counts are summarized: **PASS**
- zero-frame request is rejected: **PASS**
- invalid CLI index is rejected: **PASS**
- previous RF-001 through RF-003 tests remain green: **PASS**

## Acceptance status

- consecutive physical-camera samples acquired: **PASS**
- frame sequence increases: **PASS**
- negotiated format remains active during capture: **PASS**
- controlled capture completes: **PASS**
- local MSVC build: **PASS**
- hardware-independent tests: **PASS**
- traceability updated: **PASS**

## GitHub Actions validation

- status: completed
- conclusion: success
- commit: e4455f3b07ea7bebfce305a82eb0eb77b8123d3b
- run: https://github.com/Jeamlake/orvix/actions/runs/34082967234

The native CI job performs a clean MSVC build and executes the complete CTest
suite. The synthetic reader validates the continuous capture loop without
camera hardware; physical sample acquisition is validated separately on the
development workstation.

## Scope boundary

RF-004 counts and validates native video samples without persisting frame data.
Per-frame metadata belongs to RF-005, and effective capture-FPS measurement
belongs to RF-015.
