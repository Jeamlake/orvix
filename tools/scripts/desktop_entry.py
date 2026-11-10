"""Windowed frozen entry point; native capture remains a separate process."""
# Importa freeze_support desde multiprocessing.
from multiprocessing import freeze_support
# Importa main desde orvix.ui.desktop.
from orvix.ui.desktop import main

# Entra en esta rama si __name__ == '__main__'.
if __name__ == "__main__":
    # Ejecuta freeze_support().
    freeze_support()
    # Interrumpe la operación con SystemExit(main()).
    raise SystemExit(main())
