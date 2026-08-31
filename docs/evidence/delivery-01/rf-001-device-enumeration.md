# RF-001 — Media Foundation Device Enumeration

Status: **Validated**

Issue: **#1**

## Objective

Enumerate video capture devices from the native C++ layer using
Windows Media Foundation.

## Architecture

`	ext
orvix-capture
      |
      v
DeviceEnumerator
      |
      v
MediaFoundationDeviceEnumerator
      |
      v
Windows Media Foundation
      |
      v
Physical camera devices
`

## Implementation

The feature introduces:

- CameraDevice domain model;
- DeviceEnumerator abstraction;
- MediaFoundationDeviceEnumerator;
- RAII management for COM and Media Foundation runtime state;
- HRESULT-aware error handling;
- orvix-capture devices CLI command;
- hardware-independent native validation.

## Local validation

### CMake

`	ext
configure = PASS
build     = PASS
`

### Native tests

`	ext
CTest = PASS
`

### Development workstation runtime

`	ext
ORVIX Capture Core 0.1.0

Backend: Windows Media Foundation

Available video capture devices:

[0] GENERAL WEBCAM
    Backend: Windows Media Foundation
    Symbolic link: [REDACTED_LOCAL_DEVICE_IDENTIFIER]

Device count: 1

`

## Acceptance status

- C++ enumerates devices through Media Foundation: **PASS**
- GENERAL WEBCAM detected on development workstation: **PASS**
- Dedicated device model: **PASS**
- Media Foundation logic isolated from main.cpp: **PASS**
- Zero-device path handled without crashing: **IMPLEMENTED**
- Local build: **PASS**
- Hardware-independent tests: **PASS**
- CI: **PASS**
- Traceability updated: **PASS**

## Privacy

The repository evidence intentionally redacts the workstation-specific
camera symbolic link.

## GitHub Actions validation

The RF-001 implementation was validated by the repository CI workflow.

- status: completed
- conclusion: success
- commit: af4e298e4ddca13e5f3d0fb86a2213ed386e8c4b
- run: https://github.com/Jeamlake/orvix/actions/runs/33388027430

The CI workflow validates the hardware-independent native build and tests.
Real Media Foundation camera enumeration is validated separately on the development workstation.
