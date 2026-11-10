// Incluye las declaraciones de diagnostic error de ORVIX.
#include "orvix/capture/diagnostic_error.hpp"

// Incluye utility para usar utilidades de movimiento e intercambio.
#include <utility>

// Agrupa las declaraciones bajo orvix::capture.
namespace orvix::capture {

// Ejecuta DiagnosticError::DiagnosticError(.
DiagnosticError::DiagnosticError(
    // Declara el parámetro code.
    std::string code,
    // Declara el parámetro event.
    std::string event,
    // Declara el parámetro message.
    std::string message,
    // Incorpora const int exit_code en esta operación.
    const int exit_code
)
    // Inicializa el miembro indicado por std::runtime_error(std::move(message)), antes del cuerpo.
    : std::runtime_error(std::move(message)),
      // Transfiere el valor sin exigir una copia mediante code_.
      code_(std::move(code)),
      // Transfiere el valor sin exigir una copia mediante event_.
      event_(std::move(event)),
      // Ejecuta exit_code_(exit_code) {}.
      exit_code_(exit_code) {}

// Define la operación DiagnosticError::code con los parámetros declarados.
const std::string& DiagnosticError::code() const noexcept {
    // Devuelve code_.
    return code_;
}

// Define la operación DiagnosticError::event con los parámetros declarados.
const std::string& DiagnosticError::event() const noexcept {
    // Devuelve event_.
    return event_;
}

// Define la operación DiagnosticError::exit_code con los parámetros declarados.
int DiagnosticError::exit_code() const noexcept {
    // Devuelve exit_code_.
    return exit_code_;
}

// Incorpora } // namespace orvix::capture en esta operación.
}  // namespace orvix::capture
