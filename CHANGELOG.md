# Changelog

[English](CHANGELOG.md) | [Español](CHANGELOG.es.md)

[English](CHANGELOG.md) | [Español](CHANGELOG.es.md)

## [Unreleased]

### Added

- Versioned `ORVX` IPC v1 global header and three-slot shared-memory ring.
- C++ shared-memory producer for physical and synthetic frames.
- Camera-independent Python consumer with seqlock validation and sequence
  acknowledgement.
- NumPy/OpenCV decoding for NV12, YUY2, UYVY, I420, YV12, RGB24, BGRA32,
  ARGB32 and MJPG.
- `ORVIX Live - RAW` viewer, FPS reporting, skipped-frame detection and buffer
  utilization.
- Synthetic NV12 source and cross-process integration test on Windows, Linux
  and macOS.
- Bilingual Delivery 2 evidence and reproducible demonstration guide.

- Parallel English and Spanish documentation sections with equivalent
  architecture, requirements, platform, decision and validation records.
- Platform-neutral native camera contract and build-time backend factory.
- Linux V4L2 device enumeration, format negotiation and memory-mapped capture.
- macOS AVFoundation device enumeration, format negotiation and sample-buffer
  capture, including an embedded camera privacy description.
- Native and Python CI coverage on Windows, Ubuntu and macOS.
- Shell setup, build and test scripts for Linux, macOS and Git Bash on Windows.
- Python 3.8 compatibility path for the Windows 7 legacy target.
- Cross-platform installation, permissions and runtime documentation.

- Native camera format discovery and negotiation with an initial 1280x720 at
  30 FPS target.
- `orvix-capture formats --index <N>` native CLI command.
- Continuous Media Foundation sample acquisition for RF-004.
- `orvix-capture capture --index <N>` native CLI command.
- Hardware-independent tests for format selection and frame sequencing.
- Per-frame sequence, timestamp, resolution, pixel format and byte-size
  metadata for RF-005.
- Effective capture-FPS measurement based on Media Foundation timestamps for
  RF-015.
- Stable diagnostic codes and exit codes for missing cameras, invalid indexes,
  opening failures, stream failures, device invalidation and format changes for
  RF-023.
- Structured capture lifecycle and failure logging in
  `logs/orvix-capture.log` for RF-026.
- Delivery 1 evidence index and capture diagnostics reference.

- Media Foundation opening of the selected camera for RF-003.
- `orvix-capture open --index <N>` native CLI command.
- RAII ownership and shutdown of the active `IMFMediaSource`.

- Media Foundation device enumeration for RF-001.
- orvix-capture devices native CLI command.

- Initial monorepo architecture.
- Requirements specification.
- Architecture Decision Records.
- C++ scaffolding.
- Python scaffolding.
- Continuous Integration configuration.
