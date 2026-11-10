// Incluye las declaraciones de v4l2 device enumerator de ORVIX.
#include "orvix/capture/v4l2_device_enumerator.hpp"

// Incluye fcntl.h para usar apertura y opciones de descriptores.
#include <fcntl.h>
// Incluye linux/videodev2.h para usar tipos y operaciones de V4L2.
#include <linux/videodev2.h>
// Incluye sys/ioctl.h para usar comandos de control de dispositivos.
#include <sys/ioctl.h>
// Incluye unistd.h para usar operaciones POSIX.
#include <unistd.h>

// Incluye algorithm para usar algoritmos sobre rangos.
#include <algorithm>
// Importa las declaraciones de cctype para esta unidad.
#include <cctype>
// Incluye cerrno para usar códigos de error del sistema.
#include <cerrno>
// Incluye cstdint para usar enteros de tamaño fijo.
#include <cstdint>
// Incluye filesystem para usar rutas y operaciones de archivos.
#include <filesystem>
// Incluye string para usar cadenas de texto.
#include <string>
// Incluye vector para usar contenedores dinámicos.
#include <vector>

// Agrupa las declaraciones bajo orvix::capture.
namespace orvix::capture {

// Limita estas declaraciones a esta unidad de compilación.
namespace {

// Define la operación is_video_device_name con los parámetros declarados.
bool is_video_device_name(const std::string& name) {
    // Establece prefix a partir de "video".
    constexpr auto prefix = "video";

    // Evalúa la condición name.rfind(prefix, 0) != 0 || name.size() == 5.
    if (name.rfind(prefix, 0) != 0 || name.size() == 5) {
        // Devuelve false.
        return false;
    }

    // Devuelve std::all_of(.
    return std::all_of(
        // Ejecuta name.begin() + 5,.
        name.begin() + 5,
        // Ejecuta name.end(),.
        name.end(),
        // Envía el mensaje de Objective-C indicado por [](const unsigned char value) {.
        [](const unsigned char value) {
            // Devuelve std::isdigit(value) != 0.
            return std::isdigit(value) != 0;
        }
    );
}

// Define la operación capability_name con los parámetros declarados.
std::string capability_name(const v4l2_capability& capability) {
    // Establece begin a partir de reinterpret_cast<const char*>(capability.card).
    const auto* begin = reinterpret_cast<const char*>(capability.card);
    // Establece end a partir de std::find(begin, begin + sizeof(capability.card), '\0').
    const auto* end = std::find(begin, begin + sizeof(capability.card), '\0');
    // Devuelve std::string(begin, end).
    return std::string(begin, end);
}

// Define la operación supports_video_capture con los parámetros declarados.
bool supports_video_capture(const v4l2_capability& capability) noexcept {
    // Establece capabilities a partir de (capability.capabilities & V4L2_CAP_DEVICE_CAPS) != 0.
    const std::uint32_t capabilities =
        // Evalúa (capability.capabilities & V4L2_CAP_DEVICE_CAPS) != 0 como parte de la condición.
        (capability.capabilities & V4L2_CAP_DEVICE_CAPS) != 0
            // Incorpora ? capability.device_caps en esta operación.
            ? capability.device_caps
            // Inicializa el miembro indicado por capability.capabilities antes del cuerpo.
            : capability.capabilities;

    // Devuelve el resultado de (capabilities & V4L2_CAP_VIDEO_CAPTURE) != 0 &&.
    return
        // Combina esta condición con (capabilities & V4L2_CAP_VIDEO_CAPTURE) != 0 &&.
        (capabilities & V4L2_CAP_VIDEO_CAPTURE) != 0 &&
        // Evalúa (capabilities & V4L2_CAP_STREAMING) != 0 como parte de la condición.
        (capabilities & V4L2_CAP_STREAMING) != 0;
}

// Incorpora } // namespace en esta operación.
}  // namespace

// Define la operación V4l2DeviceEnumerator::enumerate con los parámetros declarados.
std::vector<CameraDevice> V4l2DeviceEnumerator::enumerate() const {
    // Declara paths con el tipo indicado.
    std::vector<std::filesystem::path> paths;
    // Declara error con el tipo indicado.
    std::error_code error;

    // Recorre según const auto& entry :.
    for (
        // Inicia const auto& entry : con std::filesystem::directory_iterator("/dev", error).
        const auto& entry :
            // Ejecuta std::filesystem::directory_iterator("/dev", error).
            std::filesystem::directory_iterator("/dev", error)
    ) {
        // Evalúa la condición error.
        if (error) {
            // Termina el bucle o la rama actual.
            break;
        }

        // Establece name a partir de entry.path().filename().string().
        const std::string name = entry.path().filename().string();
        // Evalúa la condición is_video_device_name(name).
        if (is_video_device_name(name)) {
            // Agrega un elemento al final del contenedor mediante paths.push_back.
            paths.push_back(entry.path());
        }
    }

    // Ejecuta std::sort(paths.begin(), paths.end()).
    std::sort(paths.begin(), paths.end());

    // Declara devices (dispositivos detectados) con el tipo indicado.
    std::vector<CameraDevice> devices;

    // Recorre según const auto& path : paths.
    for (const auto& path : paths) {
        // Establece native_path a partir de path.string().
        const std::string native_path = path.string();
        // Establece descriptor (descriptor del dispositivo) a partir de ::open(.
        const int descriptor = ::open(
            // Ejecuta native_path.c_str(),.
            native_path.c_str(),
            // Incorpora O_RDWR | O_NONBLOCK en esta operación.
            O_RDWR | O_NONBLOCK
        );

        // Evalúa la condición descriptor < 0.
        if (descriptor < 0) {
            // Evalúa la condición errno == EACCES || errno == EBUSY.
            if (errno == EACCES || errno == EBUSY) {
                // Agrega un elemento al final del contenedor mediante devices.push_back.
                devices.push_back({
                    // Ejecuta devices.size(),.
                    devices.size(),
                    // Ejecuta path.filename().string(),.
                    path.filename().string(),
                    // Aporta native_path como elemento de la lista.
                    native_path,
                    // Aporta el texto "Linux V4L2" a la expresión.
                    "Linux V4L2"
                });
            }
            // Pasa a la siguiente iteración.
            continue;
        }

        // Inicializa capability con inicialización por valor.
        v4l2_capability capability{};
        // Establece usable a partir de ::ioctl(descriptor, VIDIOC_QUERYCAP, &capability) == 0 &&.
        const bool usable =
            // Combina esta condición con ::ioctl(descriptor, VIDIOC_QUERYCAP, &capability) == 0 &&.
            ::ioctl(descriptor, VIDIOC_QUERYCAP, &capability) == 0 &&
            // Ejecuta supports_video_capture(capability).
            supports_video_capture(capability);
        // Cierra el recurso mediante close.
        static_cast<void>(::close(descriptor));

        // Evalúa la condición !usable.
        if (!usable) {
            // Pasa a la siguiente iteración.
            continue;
        }

        // Establece friendly_name a partir de capability_name(capability).
        std::string friendly_name = capability_name(capability);
        // Evalúa la condición friendly_name.empty().
        if (friendly_name.empty()) {
            // Establece friendly_name a partir de path.filename().string().
            friendly_name = path.filename().string();
        }

        // Agrega un elemento al final del contenedor mediante devices.push_back.
        devices.push_back({
            // Ejecuta devices.size(),.
            devices.size(),
            // Transfiere el valor sin exigir una copia mediante std::move.
            std::move(friendly_name),
            // Aporta native_path como elemento de la lista.
            native_path,
            // Aporta el texto "Linux V4L2" a la expresión.
            "Linux V4L2"
        });
    }

    // Devuelve devices.
    return devices;
}

// Incorpora } // namespace orvix::capture en esta operación.
}  // namespace orvix::capture
