# Native Video Format Negotiation

Status: **Validated**

Issue: **#7**

## Objective

Discover the selected camera native media types and configure the closest
available format to the initial target of 1280x720 at 30 FPS.

## Implementation

ORVIX uses `IMFSourceReader::GetNativeMediaType` to inspect the camera formats.
Each usable format records:

- native media-type index;
- width and height;
- frame-rate numerator and denominator;
- pixel format;
- whether the format is compressed.

The selection policy prioritizes:

1. exact target resolution;
2. closest frame rate;
3. efficient uncompressed pixel formats;
4. stable native media-type order.

The selected native type is applied through
`IMFSourceReader::SetCurrentMediaType` and read back for verification.

## CLI

```text
orvix-capture formats --index <N>
```

## Development workstation validation

The `GENERAL WEBCAM` exposed 15 usable native media types. Relevant options
included:

```text
[0] 1920x1080 @ 30.00 FPS, NV12
[1] 1920x1080 @ 30.00 FPS, MJPG (compressed)
[2] 1280x720 @ 30.00 FPS, NV12
[3] 1280x720 @ 30.00 FPS, MJPG (compressed)
[4] 640x360 @ 30.00 FPS, NV12
[10] 640x360 @ 30.00 FPS, YUY2
```

Negotiation result:

```text
Target resolution: 1280x720
Target FPS: 30

Negotiated resolution: 1280x720
Negotiated pixel format: NV12
Negotiated FPS: 30.00
Negotiated native type index: 2

Format status: CONFIGURED
```

## Acceptance status

- native formats detected: **PASS**
- target resolution configured: **PASS**
- target FPS configured: **PASS**
- pixel format reported: **PASS**
- configured media type read back from Media Foundation: **PASS**
- hardware-independent selection tests: **PASS**

## Scope boundary

The negotiated frame rate is the camera media-type setting. Measuring effective
capture FPS belongs to RF-015.
