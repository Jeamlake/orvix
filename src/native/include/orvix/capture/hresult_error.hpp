// Aplica la directiva del compilador once.
#pragma once

// Incluye stdexcept para usar tipos estándar de excepciones.
#include <stdexcept>
// Incluye string para usar cadenas de texto.
#include <string>

// Agrupa las declaraciones bajo orvix::capture.
namespace orvix::capture {

// Declara el tipo HResultError y las operaciones o datos de su contrato.
class HResultError final : public std::runtime_error {
// Expone los miembros de la interfaz.
public:
    // Ejecuta HResultError(long hresult, std::string operation).
    HResultError(long hresult, std::string operation);

    // Declara el atributo de compilación [[nodiscard]].
    [[nodiscard]]
    // Ejecuta long hresult() const noexcept.
    long hresult() const noexcept;

// Restringe estos miembros a la implementación.
private:
    // Declara hresult_ con el tipo indicado.
    long hresult_;
};

// Incorpora } // namespace orvix::capture en esta operación.
}  // namespace orvix::capture