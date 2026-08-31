# ADR: Multi-slot Ring Buffer

Status: **Accepted**

## Context

Capture and processing operate at different rates.

## Decision

Use multiple frame slots.

## Consequences

Frame sequence and slot ownership must be tracked.