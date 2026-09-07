# RF-003 — Open Selected Camera

Status: **Validated**

Issue: **#5**

## Objective

Open the selected video capture device from native C++ using Windows Media
Foundation.

## Architecture

```text
CameraDevice list
      |
      v
CameraSelector
      |
      v
MediaFoundationCamera
      |
      v
MFCreateDeviceSource
      |
      v
IMFMediaSource (ACTIVE)
```

## CLI

```text
orvix-capture open --index <N>
```

## Implementation

The feature introduces:

- a reusable COM runtime guard;
- a reusable Media Foundation runtime guard;
- `MediaFoundationCamera`, which owns the native `IMFMediaSource`;
- device opening through the selected camera symbolic link;
- active-source validation through `IMFMediaSource::GetCharacteristics`;
- deterministic `Shutdown` and COM release through RAII;
- controlled handling for invalid indices, incomplete descriptors and unavailable
  devices;
- hardware-independent native tests.

## Development workstation validation

```text
ORVIX Capture Core 0.1.0

Opened video capture device:

Index: 0
Name: GENERAL WEBCAM
Backend: Windows Media Foundation

Open status: OPEN
Media source: ACTIVE
```

## Automated validation

- initial camera state is closed: **PASS**
- repeated close is safe: **PASS**
- empty symbolic link is rejected: **PASS**
- unavailable device is rejected without a crash: **PASS**
- invalid CLI index is rejected: **PASS**
- previous RF-001 and RF-002 tests remain green: **PASS**

## Acceptance status

- selected camera opens through Media Foundation: **PASS**
- native media source is active: **PASS**
- camera ownership remains in C++: **PASS**
- resource cleanup is deterministic: **PASS**
- local build: **PASS**
- hardware-independent tests: **PASS**
- traceability updated: **PASS**

## GitHub Actions validation

The RF-003 implementation was validated by the repository CI workflow.

- status: completed
- conclusion: success
- commit: 5555af65763259d8ef6d0008079117b4b45722a8
- run: https://github.com/Jeamlake/orvix/actions/runs/34081939276

The native job performs a clean MSVC build and executes all hardware-independent
CTest tests. Physical camera opening is validated separately on the development
workstation.

## Scope boundary

RF-003 proves that the selected physical device can be opened. Media-type
negotiation and continuous sample acquisition belong to the next Delivery 1
increments.

## Privacy

The workstation-specific camera symbolic link is not stored in repository
evidence.
