# ADR: Media Foundation Backend

Status: **Accepted**

## Context

The case permits Media Foundation or DirectShow on Windows.

## Decision

Use Windows Media Foundation for the Windows implementation of the common
camera interface.

## Consequences

The Windows backend depends on Windows SDK APIs. Linux and macOS compile their
own native backends without this dependency, as defined by ADR-0012.
