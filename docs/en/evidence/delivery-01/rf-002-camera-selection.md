# RF-002 - Camera Selection

Status: **Validated**

Issue: **#3 1**

## Objective

Select one video capture device from the list previously enumerated by ORVIX Capture.

## Architecture

Camera selection is independent from the Media Foundation implementation:

CameraDevice list -> CameraSelector -> selected CameraDevice

The selected device is not opened in RF-002. Opening the device belongs to RF-003.

## CLI

orvix-capture select --index <N>

## Development workstation validation

ORVIX Capture Core 0.1.0

Selected video capture device:

Index: 0
Name: GENERAL WEBCAM
Backend: Windows Media Foundation
Symbolic link: [REDACTED_LOCAL_DEVICE_IDENTIFIER]

Selection status: READY_FOR_OPEN


## Invalid selection validation

- invalid index rejected: PASS
- diagnostic code: ORV-CAP-404
- process crash: NO

## Automated validation

- CameraSelector valid selection: PASS
- CameraSelector invalid selection: PASS
- CameraSelector empty list handling: PASS
- CTest: PASS
- CI: PASS

## Acceptance status

- Valid index selects the correct camera: PASS
- Invalid index handled safely: PASS
- Selection logic independent from Media Foundation: PASS
- GENERAL WEBCAM selected on development workstation: PASS
- Traceability updated: PASS

## GitHub Actions validation

- status: completed
- conclusion: success
- commit: dfacfc64865078a1af4a909db60a577d597dede6
- run: https://github.com/Jeamlake/orvix/actions/runs/33390474864
