# ORVIX IPC Protocol v1

Status: **Planned**

Delivery: **2**

## Planned Structure

```text
Shared Memory
|
+-- Global Header
+-- Frame Slot 0
+-- Frame Slot 1
+-- Frame Slot 2
+-- ...
```

## Planned Global Header

- magic;
- protocol version;
- width;
- height;
- stride;
- pixel format;
- target FPS;
- slot count;
- slot size;
- published slot;
- frame sequence;
- producer PID;
- producer state;
- creation timestamp;
- dropped frame count.

Proposed magic: `ORVX`

## Planned Slot Metadata

- sequence;
- timestamp;
- payload size;
- state.

Planned states: FREE, WRITING, READY, READING.

The exact binary layout and synchronization rules will be finalized in Delivery 2.