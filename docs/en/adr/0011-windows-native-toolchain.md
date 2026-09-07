# ADR: Windows Native Toolchain

Status: **Accepted for Windows**

## Context

Environment validation confirmed the installed Microsoft toolchain.

## Decision

Use MSVC, Windows SDK and CMake for Windows builds.

## Consequences

GCC is used for Linux and Apple Clang is used for macOS. MinGW is not a tested
Windows toolchain.
