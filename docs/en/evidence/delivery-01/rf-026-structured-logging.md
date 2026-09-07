# RF-026 — Structured Capture Logging

Status: **Validated**

Issue: **#13**

## Objective

Persist readable diagnostics for native capture lifecycle events and failures
without writing frame payloads to disk.

## Destination and schema

The CLI appends to `logs/orvix-capture.log` and prints that path in command
output. Every record contains an ISO-8601 UTC timestamp, severity, stable code,
event name and escaped message.

```text
timestamp=2026-09-07T...Z level=INFO code=ORV-CAP-200 event=camera_opened message="..."
timestamp=2026-09-07T...Z level=INFO code=ORV-CAP-211 event=camera_format_configured message="resolution=1280x720 pixel_format=NV12 fps=30.00 native_type_index=2"
timestamp=2026-09-07T...Z level=INFO code=ORV-CAP-220 event=capture_started message="requested_frames=120"
timestamp=2026-09-07T...Z level=INFO code=ORV-CAP-221 event=capture_completed message="captured_frames=120 first_sequence=1 last_sequence=120 total_bytes=165888000 duration_seconds=6.112 measured_fps=19.47"
timestamp=2026-09-07T...Z level=INFO code=ORV-CAP-202 event=camera_closed message="Camera resources released after capture."
```

An invalid-index validation also produced an ERROR record with code
`ORV-CAP-404` and event `invalid_camera_index`.

## Automated validation

`TC-LOG-001` writes informational and error records to an isolated temporary
file, verifies their level/code/event fields and quote escaping, and removes the
file afterward.

## Acceptance status

- log directory created automatically: **PASS**
- records appended and flushed: **PASS**
- lifecycle and failure events recorded: **PASS**
- stable diagnostic fields: **PASS**
- multiline and quoted messages escaped: **PASS**
- frame image data omitted: **PASS**

The full code and event catalog is maintained in
[`docs/en/observability/capture-diagnostics.md`](../../observability/capture-diagnostics.md).
