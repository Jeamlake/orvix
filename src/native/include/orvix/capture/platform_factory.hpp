// Aplica la directiva del compilador once.
#pragma once

// Incluye las declaraciones de camera de ORVIX.
#include "orvix/capture/camera.hpp"
// Incluye las declaraciones de device enumerator de ORVIX.
#include "orvix/capture/device_enumerator.hpp"

// Incluye memory para usar punteros inteligentes y gestión de memoria.
#include <memory>
// Incluye string_view para usar vistas de cadenas.
#include <string_view>

// Agrupa las declaraciones bajo orvix::capture.
namespace orvix::capture {

// Declara el atributo de compilación [[nodiscard]].
[[nodiscard]]
// Ejecuta std::unique_ptr<DeviceEnumerator> create_platform_device_enumerator().
std::unique_ptr<DeviceEnumerator> create_platform_device_enumerator();

// Declara el atributo de compilación [[nodiscard]].
[[nodiscard]]
// Ejecuta std::unique_ptr<Camera> create_platform_camera().
std::unique_ptr<Camera> create_platform_camera();

// Declara el atributo de compilación [[nodiscard]].
[[nodiscard]]
// Ejecuta std::string_view platform_backend_name() noexcept.
std::string_view platform_backend_name() noexcept;

// Incorpora } // namespace orvix::capture en esta operación.
}  // namespace orvix::capture
