// Aplica la directiva del compilador once.
#pragma once

// Agrupa las declaraciones bajo orvix::capture.
namespace orvix::capture {

// Declara el tipo ComRuntime y las operaciones o datos de su contrato.
class ComRuntime final {
// Expone los miembros de la interfaz.
public:
    // Ejecuta ComRuntime().
    ComRuntime();
    // Define ~ComRuntime para cerrar los recursos al destruir la instancia.
    ~ComRuntime();

    // Prohíbe esta operación en tiempo de compilación.
    ComRuntime(const ComRuntime&) = delete;
    // Establece operator a partir de (const ComRuntime&) = delete.
    ComRuntime& operator=(const ComRuntime&) = delete;
    // Prohíbe esta operación en tiempo de compilación.
    ComRuntime(ComRuntime&&) = delete;
    // Establece operator a partir de (ComRuntime&&) = delete.
    ComRuntime& operator=(ComRuntime&&) = delete;

// Restringe estos miembros a la implementación.
private:
    // Inicializa initialized_ con false.
    bool initialized_{false};
};

// Declara el tipo MediaFoundationRuntime y las operaciones o datos de su contrato.
class MediaFoundationRuntime final {
// Expone los miembros de la interfaz.
public:
    // Ejecuta MediaFoundationRuntime().
    MediaFoundationRuntime();
    // Define ~MediaFoundationRuntime para cerrar los recursos al destruir la instancia.
    ~MediaFoundationRuntime();

    // Prohíbe esta operación en tiempo de compilación.
    MediaFoundationRuntime(const MediaFoundationRuntime&) = delete;
    // Establece operator a partir de (const MediaFoundationRuntime&) = delete.
    MediaFoundationRuntime& operator=(const MediaFoundationRuntime&) = delete;
    // Prohíbe esta operación en tiempo de compilación.
    MediaFoundationRuntime(MediaFoundationRuntime&&) = delete;
    // Establece operator a partir de (MediaFoundationRuntime&&) = delete.
    MediaFoundationRuntime& operator=(MediaFoundationRuntime&&) = delete;

// Restringe estos miembros a la implementación.
private:
    // Inicializa initialized_ con false.
    bool initialized_{false};
};

// Incorpora } // namespace orvix::capture en esta operación.
}  // namespace orvix::capture
