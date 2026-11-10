// Aplica la directiva del compilador once.
#pragma once

// Incluye stdexcept para usar tipos estándar de excepciones.
#include <stdexcept>
// Incluye string para usar cadenas de texto.
#include <string>

// Agrupa las declaraciones bajo orvix::capture.
namespace orvix::capture {

// Declara el tipo DiagnosticError y las operaciones o datos de su contrato.
class DiagnosticError final : public std::runtime_error {
// Expone los miembros de la interfaz.
public:
    // Ejecuta DiagnosticError(.
    DiagnosticError(
        // Declara el parámetro code.
        std::string code,
        // Declara el parámetro event.
        std::string event,
        // Declara el parámetro message.
        std::string message,
        // Incorpora int exit_code en esta operación.
        int exit_code
    );

    // Declara el atributo de compilación [[nodiscard]].
    [[nodiscard]]
    // Ejecuta const std::string& code() const noexcept.
    const std::string& code() const noexcept;

    // Declara el atributo de compilación [[nodiscard]].
    [[nodiscard]]
    // Ejecuta const std::string& event() const noexcept.
    const std::string& event() const noexcept;

    // Declara el atributo de compilación [[nodiscard]].
    [[nodiscard]]
    // Ejecuta int exit_code() const noexcept.
    int exit_code() const noexcept;

// Restringe estos miembros a la implementación.
private:
    // Declara code_ con el tipo indicado.
    std::string code_;
    // Declara event_ con el tipo indicado.
    std::string event_;
    // Declara exit_code_ con el tipo indicado.
    int exit_code_;
};

// Incorpora } // namespace orvix::capture en esta operación.
}  // namespace orvix::capture
