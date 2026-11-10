// Aplica la directiva del compilador once.
#pragma once

// Agrupa las declaraciones bajo orvix::cli.
namespace orvix::cli {

// Declara el tipo CaptureCli y las operaciones o datos de su contrato.
class CaptureCli final {
// Expone los miembros de la interfaz.
public:
    // Declara el atributo de compilación [[nodiscard]].
    [[nodiscard]]
    // Ejecuta int run(int argc, char* argv[]) const.
    int run(int argc, char* argv[]) const;
};

// Incorpora } // namespace orvix::cli en esta operación.
}  // namespace orvix::cli
