# Protocolo IPC v1 de ORVIX

[English](../../en/protocol/ipc-v1.md) | [Español](ipc-v1.md)

Estado: **Planificado**

Entrega: **2**

## Estructura planificada

```text
Memoria compartida
|
+-- Header global
+-- Slot de frame 0
+-- Slot de frame 1
+-- Slot de frame 2
+-- ...
```

## Header global planificado

- magic;
- versión del protocolo;
- ancho;
- alto;
- stride;
- formato de píxel;
- FPS objetivo;
- cantidad de slots;
- tamaño de slot;
- slot publicado;
- secuencia del frame;
- PID del productor;
- estado del productor;
- timestamp de creación;
- cantidad de frames perdidos.

Magic propuesto: `ORVX`

## Metadatos planificados por slot

- secuencia;
- timestamp;
- tamaño del payload;
- estado.

Estados previstos: `FREE`, `WRITING`, `READY`, `READING`.

El diseño binario y las reglas de sincronización se definirán en la Entrega 2.
