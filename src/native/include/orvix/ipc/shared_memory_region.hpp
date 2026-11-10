// Aplica la directiva del compilador once.
#pragma once

// Incluye cstddef para usar tipos de tamaño y bytes.
#include <cstddef>
// Incluye memory para usar punteros inteligentes y gestión de memoria.
#include <memory>
// Incluye string para usar cadenas de texto.
#include <string>

// Agrupa las declaraciones bajo orvix::ipc.
namespace orvix::ipc {

// Declara el tipo SharedMemoryRegion y las operaciones o datos de su contrato.
class SharedMemoryRegion final {
// Expone los miembros de la interfaz.
public:
    // Ejecuta SharedMemoryRegion(std::string name, std::size_t size).
    SharedMemoryRegion(std::string name, std::size_t size);
    // Define ~SharedMemoryRegion para cerrar los recursos al destruir la instancia.
    ~SharedMemoryRegion();

    // Prohíbe esta operación en tiempo de compilación.
    SharedMemoryRegion(const SharedMemoryRegion&) = delete;
    // Establece operator a partir de (const SharedMemoryRegion&) = delete.
    SharedMemoryRegion& operator=(const SharedMemoryRegion&) = delete;
    // Prohíbe esta operación en tiempo de compilación.
    SharedMemoryRegion(SharedMemoryRegion&&) = delete;
    // Establece operator a partir de (SharedMemoryRegion&&) = delete.
    SharedMemoryRegion& operator=(SharedMemoryRegion&&) = delete;

    // Declara el atributo de compilación [[nodiscard]].
    [[nodiscard]]
    // Ejecuta std::byte* data() noexcept.
    std::byte* data() noexcept;

    // Declara el atributo de compilación [[nodiscard]].
    [[nodiscard]]
    // Ejecuta const std::byte* data() const noexcept.
    const std::byte* data() const noexcept;

    // Declara el atributo de compilación [[nodiscard]].
    [[nodiscard]]
    // Ejecuta std::size_t size() const noexcept.
    std::size_t size() const noexcept;

    // Declara el atributo de compilación [[nodiscard]].
    [[nodiscard]]
    // Ejecuta const std::string& name() const noexcept.
    const std::string& name() const noexcept;

// Restringe estos miembros a la implementación.
private:
    // Declara el tipo Impl y las operaciones o datos de su contrato.
    struct Impl;
    // Declara impl_ con el tipo indicado.
    std::unique_ptr<Impl> impl_;
};

// Incorpora } // namespace orvix::ipc en esta operación.
}  // namespace orvix::ipc
