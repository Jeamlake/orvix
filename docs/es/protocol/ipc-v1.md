# Protocolo IPC v1 de ORVIX

[English](../../en/protocol/ipc-v1.md) | [Español](ipc-v1.md)

Estado: **Implementado**

Entrega: **2**

## Propósito

El proceso C++ es el único propietario de la cámara y publica cada frame en
memoria compartida. Python se conecta por nombre, copia el último slot estable,
interpreta el formato con NumPy y lo muestra mediante OpenCV. No se crean
archivos temporales ni Python abre la cámara.

## Organización

```text
Header global: 128 bytes
Slot 0: header de 64 bytes + payload
Slot 1: header de 64 bytes + payload
Slot 2: header de 64 bytes + payload
```

El nombre predeterminado es `orvix-camera-v1`. Los enteros usan little-endian y
todos los offsets multibyte están alineados naturalmente.

## Header global

| Offset | Tamaño | Tipo | Campo |
|---:|---:|---|---|
| 0 | 4 | bytes | Magic `ORVX` |
| 4 | 2 | uint16 | Versión, valor `1` |
| 6 | 2 | uint16 | Tamaño del header, `128` |
| 8 | 8 | uint64 | Tamaño total del mapeo |
| 16 | 4 | uint32 | Cantidad de slots |
| 20 | 4 | uint32 | Tamaño del header de slot, `64` |
| 24 | 8 | uint64 | Capacidad de payload por slot |
| 32 | 8 | uint64 | Distancia entre slots |
| 40 | 4 | uint32 | Ancho configurado |
| 44 | 4 | uint32 | Alto configurado |
| 48 | 4 | uint32 | Stride del frame |
| 52 | 4 | uint32 | Código de formato de píxel |
| 56 | 4 | uint32 | Numerador de FPS |
| 60 | 4 | uint32 | Denominador de FPS |
| 64 | 8 | int64 | Creación, en unidades de 100 ns |
| 72 | 8 | uint64 | PID del productor |
| 80 | 4 | uint32 | Estado del productor |
| 84 | 4 | uint32 | Último slot publicado |
| 88 | 8 | uint64 | Última secuencia publicada |
| 96 | 8 | uint64 | Última secuencia consumida |
| 104 | 8 | uint64 | Frames sobrescritos |
| 112 | 8 | uint64 | FPS del productor multiplicados por 1000 |
| 120 | 8 | bytes | Reservado |

Estados del productor: `0 INITIALIZING`, `1 RUNNING`, `2 STOPPED` y `3 FAILED`.

## Header de cada slot

| Offset | Tamaño | Tipo | Campo |
|---:|---:|---|---|
| 0 | 8 | uint64 | Generación para seqlock |
| 8 | 8 | uint64 | Secuencia |
| 16 | 8 | int64 | Timestamp en unidades de 100 ns |
| 24 | 8 | uint64 | Bytes válidos del payload |
| 32 | 4 | uint32 | Ancho |
| 36 | 4 | uint32 | Alto |
| 40 | 4 | uint32 | Stride |
| 44 | 4 | uint32 | Código de formato de píxel |
| 48 | 4 | uint32 | Estado del slot |
| 52 | 12 | bytes | Reservado |
| 64 | variable | bytes | Payload del frame |

Estados del slot: `0 EMPTY`, `1 WRITING` y `2 READY`. Los códigos de formato son
`NV12=1`, `YUY2=2`, `UYVY=3`, `I420=4`, `YV12=5`, `RGB24=6`, `BGRA32=7`,
`ARGB32=8`, `MJPG=9` y `H264=10`.

## Sincronización

El productor vuelve impar la generación del slot, escribe metadatos y payload,
marca el slot como listo y publica una generación par. Después actualiza el
índice y la secuencia global. El consumidor lee la generación antes y después
de copiar; acepta el frame solamente si ambos valores coinciden y son pares.
Luego escribe la secuencia consumida en el offset 96.

El productor nunca espera a Python. Cuando el consumidor queda atrás, el anillo
conserva los tres frames más recientes y contabiliza las sobrescrituras. Esta
política mantiene baja la latencia y permite que Capture continúe si Vision se
cierra o falla.

## Ciclo de vida

El productor crea el mapeo de forma exclusiva, publica `RUNNING`, emite frames y
finaliza con `STOPPED` o `FAILED`. En Windows usa `CreateFileMapping`; en Linux y
macOS usa `shm_open` y `mmap`. El mapeo se libera al salir el productor; un
consumidor que ya estaba conectado puede terminar de leer su vista activa.
