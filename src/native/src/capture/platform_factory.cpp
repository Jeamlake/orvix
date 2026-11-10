// Incluye las declaraciones de platform factory de ORVIX.
#include "orvix/capture/platform_factory.hpp"

// Incluye este bloque si se cumple defined(_WIN32).
#if defined(_WIN32)
// Incluye las declaraciones de media foundation camera de ORVIX.
#include "orvix/capture/media_foundation_camera.hpp"
// Incluye las declaraciones de media foundation device enumerator de ORVIX.
#include "orvix/capture/media_foundation_device_enumerator.hpp"
// Evalúa la condición de compilación alternativa defined(__APPLE__).
#elif defined(__APPLE__)
// Incluye las declaraciones de av foundation camera de ORVIX.
#include "orvix/capture/av_foundation_camera.hpp"
// Incluye las declaraciones de av foundation device enumerator de ORVIX.
#include "orvix/capture/av_foundation_device_enumerator.hpp"
// Evalúa la condición de compilación alternativa defined(__linux__).
#elif defined(__linux__)
// Incluye las declaraciones de v4l2 camera de ORVIX.
#include "orvix/capture/v4l2_camera.hpp"
// Incluye las declaraciones de v4l2 device enumerator de ORVIX.
#include "orvix/capture/v4l2_device_enumerator.hpp"
// Abre la alternativa de compilación.
#else
// Aplica la directiva error "ORVIX does not have a native capture backend for this platform.".
#error "ORVIX does not have a native capture backend for this platform."
// Cierra la condición de compilación.
#endif

// Incluye memory para usar punteros inteligentes y gestión de memoria.
#include <memory>
// Incluye string_view para usar vistas de cadenas.
#include <string_view>

// Agrupa las declaraciones bajo orvix::capture.
namespace orvix::capture {

// Define la operación create_platform_device_enumerator con los parámetros declarados.
std::unique_ptr<DeviceEnumerator> create_platform_device_enumerator() {
// Incluye este bloque si se cumple defined(_WIN32).
#if defined(_WIN32)
    // Devuelve std::make_unique<MediaFoundationDeviceEnumerator>().
    return std::make_unique<MediaFoundationDeviceEnumerator>();
// Evalúa la condición de compilación alternativa defined(__APPLE__).
#elif defined(__APPLE__)
    // Devuelve std::make_unique<AvFoundationDeviceEnumerator>().
    return std::make_unique<AvFoundationDeviceEnumerator>();
// Evalúa la condición de compilación alternativa defined(__linux__).
#elif defined(__linux__)
    // Devuelve std::make_unique<V4l2DeviceEnumerator>().
    return std::make_unique<V4l2DeviceEnumerator>();
// Cierra la condición de compilación.
#endif
}

// Define la operación create_platform_camera con los parámetros declarados.
std::unique_ptr<Camera> create_platform_camera() {
// Incluye este bloque si se cumple defined(_WIN32).
#if defined(_WIN32)
    // Devuelve std::make_unique<MediaFoundationCamera>().
    return std::make_unique<MediaFoundationCamera>();
// Evalúa la condición de compilación alternativa defined(__APPLE__).
#elif defined(__APPLE__)
    // Devuelve std::make_unique<AvFoundationCamera>().
    return std::make_unique<AvFoundationCamera>();
// Evalúa la condición de compilación alternativa defined(__linux__).
#elif defined(__linux__)
    // Devuelve std::make_unique<V4l2Camera>().
    return std::make_unique<V4l2Camera>();
// Cierra la condición de compilación.
#endif
}

// Define la operación platform_backend_name con los parámetros declarados.
std::string_view platform_backend_name() noexcept {
// Incluye este bloque si se cumple defined(_WIN32).
#if defined(_WIN32)
    // Devuelve "Windows Media Foundation".
    return "Windows Media Foundation";
// Evalúa la condición de compilación alternativa defined(__APPLE__).
#elif defined(__APPLE__)
    // Devuelve "macOS AVFoundation".
    return "macOS AVFoundation";
// Evalúa la condición de compilación alternativa defined(__linux__).
#elif defined(__linux__)
    // Devuelve "Linux V4L2".
    return "Linux V4L2";
// Cierra la condición de compilación.
#endif
}

// Incorpora } // namespace orvix::capture en esta operación.
}  // namespace orvix::capture
