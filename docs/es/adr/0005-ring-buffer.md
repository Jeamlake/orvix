# ADR: Búfer circular con múltiples slots

[English](../../en/adr/0005-ring-buffer.md) | [Español](0005-ring-buffer.md)

Estado: **Aceptado**

## Contexto

La captura y el procesamiento operan a velocidades diferentes.

## Decisión

Utilizar múltiples slots para frames.

## Consecuencias

Se deben controlar la secuencia y la propiedad de cada slot.
