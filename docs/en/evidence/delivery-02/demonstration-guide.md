# Delivery 2 Demonstration Guide

[English](demonstration-guide.md) | [Español](../../../es/evidence/delivery-02/demonstration-guide.md)

## One-time setup

From Git Bash in the ORVIX root:

```bash
./scripts/setup.sh
./scripts/build.sh
./scripts/test.sh
```

PowerShell users can run `scripts\setup.ps1`, `scripts\build.ps1` and
`scripts\test.ps1`.

## Physical webcam demonstration

Open two terminals in the repository root. In the first terminal:

```bash
./build/native/Release/orvix-capture.exe bridge --index 0 --frames 900
```

In the second Git Bash terminal:

```bash
source .venv/Scripts/activate
python -m orvix.ui.viewer
```

In PowerShell:

```powershell
.\.venv\Scripts\Activate.ps1
python -m orvix.ui.viewer
```

On Linux or macOS, run the producer as `./build/native/orvix-capture` and
activate the environment with `source .venv/bin/activate`.

A window titled **ORVIX Live - RAW** displays physical video. Its overlay shows
sequence, resolution, format, producer FPS, viewer FPS and skipped frames.
Pressing `Q` or `Esc` closes Vision only; Capture continues until it reaches the
requested frame count.

## Demonstration without a webcam

The same path can use synthetic frames. In the first terminal:

```bash
./build/native/Release/orvix-capture.exe bridge --synthetic --frames 900
```

Run the same viewer command in the second terminal. The window displays an
animated gradient. This path verifies the protocol, synchronization, NumPy and
OpenCV without physical hardware.

## Headless verification

For a quick check or CI environment:

```bash
python -m orvix.ui.viewer --headless --frames 30
```

Successful output includes `Viewer status: COMPLETED`, a positive frame count
and sequence, overwrite and utilization metrics.
