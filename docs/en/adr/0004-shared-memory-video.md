# ADR: Shared Memory Video Transport

Status: **Accepted**

## Context

Video frames are large payloads already resident in memory.

## Decision

Use shared memory between native and Python processes.

## Consequences

ORVIX must define synchronization and a binary IPC contract.