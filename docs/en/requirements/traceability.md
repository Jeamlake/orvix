# ORVIX Requirements Traceability Matrix

| Requirement | Component | Delivery | Test | Status |
|---|---|---:|---|---|
| RF-001 | Capture | 1 | TC-CAP-001 | Verified |
| RF-002 | Capture | 1 | TC-CAP-002 | Verified |
| RF-003 | Capture | 1 | TC-CAP-003 | Verified |
| RF-004 | Capture | 1 | TC-CAP-004 | Verified |
| RF-005 | Capture | 1 | TC-CAP-005 | Verified |
| RF-006 | Bridge | 2 | TC-IPC-001 | Verified |
| RF-007 | Vision | 2 | TC-IPC-005 | Verified |
| RF-008 | Vision | 2 | TC-IPC-006 | Verified |
| RF-009 | Vision | 2 | TC-VIS-001 | Verified |
| RF-010 | Vision | 2 | TC-VIS-001 | Verified |
| RF-011 | Vision | 3 | Pending | Planned |
| RF-012 | Vision | 3 | Pending | Planned |
| RF-013 | Vision | 3 | TC-VIS-004 | Planned |
| RF-014 | Vision | 3 | Pending | Planned |
| RF-015 | Insight | 1 | TC-MET-001 | Verified |
| RF-016 | Insight | 3 | Pending | Planned |
| RF-017 | Bridge | 2 | TC-IPC-008 | Verified |
| RF-018 | Insight | 3 | Pending | Planned |
| RF-019 | Insight | 3 | TC-MET-004 | Planned |
| RF-020 | Insight | 2 | TC-MET-005 | Verified |
| RF-021 | Insight | 3 | TC-DB-001 | Planned |
| RF-022 | Insight | 3 | Pending | Planned |
| RF-023 | Capture | 1 | TC-CAP-023 | Verified |
| RF-024 | Capture / Vision | 2 | TC-IPC-005 | Verified |
| RF-025 | Test Infrastructure | 2 | TC-SYN-001 | Verified |
| RF-026 | All | 1 | TC-LOG-001 | Verified |
| RF-027 | Capture | 1 | TC-CAP-003-PLATFORM | Verified |

Delivery 2 desktop tests in `python/tests/test_desktop.py` cover layout,
start/stop/restart, native failures, invalid indexes, startup cancellation and
consumer reconnection without restarting Capture. Validation scope and limits
are recorded in [desktop-validation.md](../evidence/delivery-02/desktop-validation.md).
