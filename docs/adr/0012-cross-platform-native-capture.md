# ADR: Cross-Platform Native Capture Backends

Status: **Accepted**

## Context

ORVIX must run on Windows, Linux and macOS while preserving the architectural
rule that C++ owns the camera. Media Foundation cannot provide capture outside
Windows, and routing the normal path through Python would violate RF-ARCH-001.

## Decision

Define a platform-neutral C++ `Camera` contract and select one native backend at
build time:

- Windows: Media Foundation;
- Linux: Video4Linux2;
- macOS: AVFoundation.

Device enumeration is also exposed through a common contract. The CLI obtains
both objects from a platform factory and contains no backend-specific includes.
Sequence assignment, metadata, FPS calculation, diagnostics and logging remain
shared.

## Consequences

CMake compiles and links only the backend for the host operating system. CI
must build and run hardware-independent tests on all three platforms. Physical
camera validation remains platform-specific because hosted CI runners do not
expose webcams.

Windows 7 SP1 uses the Media Foundation implementation with the Win32 API
target restricted to version 6.1 and a static MSVC runtime. Its legacy Python
path uses Python 3.8.
