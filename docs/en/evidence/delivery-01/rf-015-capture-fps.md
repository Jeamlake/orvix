# RF-015 — Effective Capture FPS

Status: **Validated**

Issue: **#11**

## Objective

Measure the effective rate of accepted camera frames independently from the
frame rate requested during format negotiation.

## Calculation

For a capture containing at least two samples:

```text
elapsed_seconds = (last_timestamp_100ns - first_timestamp_100ns) / 10,000,000
capture_fps = (captured_frames - 1) / elapsed_seconds
```

The numerator uses frame intervals, so a 120-frame capture measures 119 elapsed
intervals. The timestamps come from Media Foundation and use 100-nanosecond
units.

## Physical-camera validation

The development workstation produced:

```text
Negotiated resolution: 1280x720
Negotiated pixel format: NV12
Negotiated FPS: 30.00
Frames captured: 120
Capture duration: 6.112 s
Measured capture FPS: 19.47
Capture status: CAPTURE_COMPLETED
```

The media type was configured for 30 FPS, while the effective rate observed in
this run was 19.47 FPS. ORVIX reports that difference instead of presenting the
negotiated setting as a measurement.

## Automated validation

`TC-MET-001` supplies three synthetic frames spaced 333,333 timestamp units
apart and verifies a calculated result of 30 FPS within a small floating-point
tolerance.

## Acceptance status

- elapsed media time calculated: **PASS**
- effective FPS calculated from accepted frames: **PASS**
- negotiated and measured FPS reported separately: **PASS**
- physical measurement completed: **PASS**
