# Native Capture Diagnostics

ORVIX reports expected native capture failures with a stable code, a readable
message, a structured log event and a non-zero process exit code. The log file
is `logs/orvix-capture.log`, relative to the process working directory.

## Log format

Each record occupies one line:

```text
timestamp=<UTC ISO-8601> level=<LEVEL> code=<CODE> event=<EVENT> message="<DETAILS>"
```

Messages escape quotes and backslashes and replace line breaks with spaces so a
record cannot accidentally span multiple lines. The logger appends and flushes
each record immediately.

## Diagnostic catalog

| Code | Event | Exit | Meaning |
|---|---|---:|---|
| ORV-CAP-100 | `camera_absent` | 65 | No video capture device is available for a command that requires one. |
| ORV-CAP-404 | `invalid_camera_index` | 66 | The selected index is outside the enumerated device list. |
| ORV-CAP-501 | `camera_open_failed` | 71 | Media Foundation could not activate the selected camera. |
| ORV-CAP-502 | `format_negotiation_failed` | 72 | No usable media type could be configured. |
| ORV-CAP-503 | `camera_disconnected` | 74 | The device was removed or Media Foundation invalidated it. |
| ORV-CAP-503 | `camera_stream_error` | 74 | The source reader reported a stream error. |
| ORV-CAP-503 | `camera_stream_ended` | 74 | The stream ended before the requested capture completed. |
| ORV-CAP-503 | `camera_stream_stalled` | 74 | The camera exceeded the allowed number of reads without a frame. |
| ORV-CAP-504 | `frame_read_failed` | 75 | A frame read failed for another native API reason. |
| ORV-CAP-505 | `camera_format_changed` | 75 | The active media type changed during the bounded capture. |
| ORV-CAP-400 | `invalid_argument` | 64 | A CLI argument is malformed. |
| ORV-CAP-400 | `invalid_command` | 64 | The command shape is unsupported. |
| ORV-CAP-500 | `native_api_failure` | 70 | An uncategorized native capture API call failed. |
| ORV-CAP-599 | `unexpected_failure` | 70 | An uncategorized application exception escaped the command. |

`orvix-capture devices` treats an empty device list as a successful enumeration
with a warning and a device count of zero. Commands that require a camera return
`ORV-CAP-100` instead.

## Lifecycle events

Successful operations use informational codes from `ORV-CAP-101` through
`ORV-CAP-221` for enumeration, selection, opening, format discovery,
configuration, capture start, capture completion and resource release.

The capture completion record includes captured frame count, first and last
sequence, total bytes, elapsed media time and measured FPS. Image data is never
written to the log.
