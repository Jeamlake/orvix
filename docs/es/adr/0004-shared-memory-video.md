# ADR: Transporte de video mediante memoria compartida

[English](../../en/adr/0004-shared-memory-video.md) | [Español](0004-shared-memory-video.md)

Estado: **Aceptado**

## Contexto

Los frames de video son cargas grandes que ya residen en memoria.

## Decisión

Utilizar memoria compartida entre los procesos nativo y Python.

## Consecuencias

ORVIX debe definir sincronización y un contrato IPC binario.
