# Importa orvix.
import orvix

# Comprueba version en la suite de pruebas.
def test_version():
    # Verifica que se cumpla orvix.__version__ == '1.0.0'.
    assert orvix.__version__ == "1.0.0"
