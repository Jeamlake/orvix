# ADR: C++ administra la cámara

[English](../../en/adr/0003-cpp-owns-camera.md) | [Español](0003-cpp-owns-camera.md)

Estado: **Aceptado**

## Contexto

El acceso directo desde Python omitiría la arquitectura nativa de E/S.

## Decisión

Solo ORVIX Capture abre la webcam física durante la operación normal.

## Consecuencias

Python recibe los frames mediante ORVIX Bridge.
