#!/usr/bin/env sh
# Configura las opciones de ejecución del script con set -u.
set -u

# Establece ROOT con $(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd).
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
# Establece INDEX con ${1:-0}.
INDEX=${1:-0}
# Establece FRAMES con ${2:-900}.
FRAMES=${2:-900}
# Establece NAME con "orvix-camera-v1".
NAME="orvix-camera-v1"

# Selecciona las opciones del script según "$(uname -s)".
case "$(uname -s)" in
    # Selecciona las plataformas que coinciden con MINGW*|MSYS*|CYGWIN*.
    MINGW*|MSYS*|CYGWIN*)
        # Establece CAPTURE con "$ROOT/build/native/Release/orvix-capture.exe".
        CAPTURE="$ROOT/build/native/Release/orvix-capture.exe"
        # Establece PYTHON con "$ROOT/.venv/Scripts/python.exe".
        PYTHON="$ROOT/.venv/Scripts/python.exe"
        ;;
    # Ejecuta la alternativa cuando no coincide el caso anterior.
    *)
        # Establece CAPTURE con "$ROOT/build/native/orvix-capture".
        CAPTURE="$ROOT/build/native/orvix-capture"
        # Establece PYTHON con "$ROOT/.venv/bin/python".
        PYTHON="$ROOT/.venv/bin/python"
        ;;
esac

# Comprueba if [ ! -f "$CAPTURE" ].
if [ ! -f "$CAPTURE" ]; then
    # Muestra 'ORVIX is not built. Run ./tools/scripts/build.sh first.\n' >&2 en la salida indicada.
    printf 'ORVIX is not built. Run ./tools/scripts/build.sh first.\n' >&2
    # Finaliza con el código 1.
    exit 1
fi

# Comprueba if [ ! -f "$PYTHON" ].
if [ ! -f "$PYTHON" ]; then
    # Muestra 'The Python environment is missing. Run ./tools/scripts/setup.sh first.\n' >&2 en la salida indicada.
    printf 'The Python environment is missing. Run ./tools/scripts/setup.sh first.\n' >&2
    # Finaliza con el código 1.
    exit 1
fi

# Establece PRODUCER_PID con "".
PRODUCER_PID=""

# Define la función del script stop_producer.
stop_producer() {
    # Comprueba if [ -n "$PRODUCER_PID" ].
    if [ -n "$PRODUCER_PID" ]; then
        # Solicita terminar el proceso indicado por kill "$PRODUCER_PID" 2>/dev/null || true.
        kill "$PRODUCER_PID" 2>/dev/null || true
        # Espera la salida de el proceso indicado por wait "$PRODUCER_PID" 2>/dev/null || true.
        wait "$PRODUCER_PID" 2>/dev/null || true
    fi
}

# Configura el manejo de señales con stop_producer EXIT INT TERM.
trap stop_producer EXIT INT TERM

# Inicia el puente nativo en segundo plano para que el visor se conecte.
# --index selecciona la cámara, --frames limita la captura y --name identifica la región compartida.
"$CAPTURE" bridge \
    --index "$INDEX" \
    --frames "$FRAMES" \
    --name "$NAME" &
# Establece PRODUCER_PID con $!.
PRODUCER_PID=$!

# Incorpora "$PYTHON" -m orvix.ui.viewer --name "$NAME" en esta operación.
"$PYTHON" -m orvix.ui.viewer --name "$NAME"
# Establece VIEWER_STATUS con $?.
VIEWER_STATUS=$?

# Incorpora stop_producer en esta operación.
stop_producer
# Establece PRODUCER_PID con "".
PRODUCER_PID=""
# Configura el manejo de señales con - EXIT INT TERM.
trap - EXIT INT TERM

# Finaliza con el código "$VIEWER_STATUS".
exit "$VIEWER_STATUS"
