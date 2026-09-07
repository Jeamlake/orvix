# ADR: Backend Media Foundation

[English](../../en/adr/0002-media-foundation-backend.md) | [Español](0002-media-foundation-backend.md)

Estado: **Aceptado**

## Contexto

El caso permite usar Media Foundation o DirectShow en Windows.

## Decisión

Utilizar Windows Media Foundation para implementar en Windows la interfaz común
de cámara.

## Consecuencias

El backend de Windows depende de las API de Windows SDK. Linux y macOS compilan
sus propios backends nativos sin esta dependencia, según ADR-0012.
