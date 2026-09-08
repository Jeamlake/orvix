#!/usr/bin/env sh
set -u

ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
INDEX=${1:-0}
FRAMES=${2:-900}
NAME="orvix-camera-v1"

case "$(uname -s)" in
    MINGW*|MSYS*|CYGWIN*)
        CAPTURE="$ROOT/build/native/Release/orvix-capture.exe"
        PYTHON="$ROOT/.venv/Scripts/python.exe"
        ;;
    *)
        CAPTURE="$ROOT/build/native/orvix-capture"
        PYTHON="$ROOT/.venv/bin/python"
        ;;
esac

if [ ! -f "$CAPTURE" ]; then
    printf 'ORVIX is not built. Run ./scripts/build.sh first.\n' >&2
    exit 1
fi

if [ ! -f "$PYTHON" ]; then
    printf 'The Python environment is missing. Run ./scripts/setup.sh first.\n' >&2
    exit 1
fi

PRODUCER_PID=""

stop_producer() {
    if [ -n "$PRODUCER_PID" ]; then
        kill "$PRODUCER_PID" 2>/dev/null || true
        wait "$PRODUCER_PID" 2>/dev/null || true
    fi
}

trap stop_producer EXIT INT TERM

"$CAPTURE" bridge \
    --index "$INDEX" \
    --frames "$FRAMES" \
    --name "$NAME" &
PRODUCER_PID=$!

"$PYTHON" -m orvix.ui.viewer --name "$NAME"
VIEWER_STATUS=$?

stop_producer
PRODUCER_PID=""
trap - EXIT INT TERM

exit "$VIEWER_STATUS"
