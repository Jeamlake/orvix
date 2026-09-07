# Changelog

## [Unreleased]

### Added

- Native camera format discovery and negotiation with an initial 1280x720 at
  30 FPS target.
- `orvix-capture formats --index <N>` native CLI command.
- Continuous Media Foundation sample acquisition for RF-004.
- `orvix-capture capture --index <N>` native CLI command.
- Hardware-independent tests for format selection and frame sequencing.

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
