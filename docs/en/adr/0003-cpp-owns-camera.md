# ADR: C++ Owns Camera

Status: **Accepted**

## Context

Direct camera access from Python would bypass the native I/O architecture.

## Decision

Only ORVIX Capture opens the physical webcam during normal operation.

## Consequences

Python receives frames through ORVIX Bridge.