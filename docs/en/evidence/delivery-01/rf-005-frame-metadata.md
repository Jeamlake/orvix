# RF-005 — Frame Metadata

Status: **Validated**

Issue: **#10**

## Objective

Expose enough metadata for every accepted native frame to identify its order,
media time, layout and payload size without persisting the image.

## Reported fields

| Field | Source |
|---|---|
| Sequence | ORVIX counter incremented after each accepted sample. |
| Timestamp | Media Foundation sample timestamp in 100-nanosecond units. |
| Resolution | Media type configured on the source reader. |
| Pixel format | Media subtype configured on the source reader. |
| Byte count | Total contiguous sample-buffer length. |

## Physical-camera validation

Command:

```powershell
./build/native/Release/orvix-capture.exe capture --index 0
```

Representative output:

```text
Frame #1 | timestamp=... (100 ns) | resolution=1280x720 | format=NV12 | bytes=1382400
Frame #2 | timestamp=... (100 ns) | resolution=1280x720 | format=NV12 | bytes=1382400
...
Frame #120 | timestamp=... (100 ns) | resolution=1280x720 | format=NV12 | bytes=1382400

Frames captured: 120
First sequence: 1
Last sequence: 120
Captured bytes: 165888000
Sequence status: STRICTLY_INCREASING
```

Each NV12 payload has the expected 1,382,400 bytes for 1280x720 at 12 bits per
pixel. The complete bounded run reported metadata for all 120 accepted samples.

## Automated validation

`TC-CAP-005` uses a synthetic frame reader and verifies the second frame's
sequence, timestamp, 1280x720 resolution, NV12 format and byte count. It also
checks the aggregate frame count and total bytes.

## Acceptance status

- per-frame sequence exposed: **PASS**
- Media Foundation timestamp exposed: **PASS**
- resolution and pixel format exposed: **PASS**
- sample byte count exposed: **PASS**
- frames remain ephemeral: **PASS**
