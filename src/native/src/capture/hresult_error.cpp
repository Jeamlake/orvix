// Incluye las declaraciones de hresult error de ORVIX.
#include "orvix/capture/hresult_error.hpp"

// Incluye iomanip para usar formato de flujos.
#include <iomanip>
// Incluye sstream para usar flujos de texto en memoria.
#include <sstream>
// Incluye utility para usar utilidades de movimiento e intercambio.
#include <utility>

// Agrupa las declaraciones bajo orvix::capture.
namespace orvix::capture {

// Limita estas declaraciones a esta unidad de compilación.
namespace {

// Define la operación build_message con los parámetros declarados.
std::string build_message(const long hresult, const std::string& operation) {
    // Declara stream con el tipo indicado.
    std::ostringstream stream;

    // Incorpora stream << operation en esta operación.
    stream << operation
           // Incorpora << " failed with HRESULT 0x" en esta operación.
           << " failed with HRESULT 0x"
           // Incorpora << std::uppercase en esta operación.
           << std::uppercase
           // Incorpora << std::hex en esta operación.
           << std::hex
           // Incorpora << static_cast<unsigned long>(hresult) en esta operación.
           << static_cast<unsigned long>(hresult);

    // Devuelve stream.str().
    return stream.str();
}

// Incorpora } // namespace en esta operación.
}  // namespace

// Ejecuta HResultError::HResultError(const long hresult, std::string operation).
HResultError::HResultError(const long hresult, std::string operation)
    // Inicializa el miembro indicado por std::runtime_error(build_message(hresult, operation)), antes del cuerpo.
    : std::runtime_error(build_message(hresult, operation)),
      // Ejecuta hresult_(hresult) {}.
      hresult_(hresult) {}

// Define la operación HResultError::hresult con los parámetros declarados.
long HResultError::hresult() const noexcept {
    // Devuelve hresult_.
    return hresult_;
}

// Incorpora } // namespace orvix::capture en esta operación.
}  // namespace orvix::capture