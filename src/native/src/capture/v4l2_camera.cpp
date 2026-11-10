// Incluye las declaraciones de v4l2 camera de ORVIX.
#include "orvix/capture/v4l2_camera.hpp"

// Incluye las declaraciones de continuous frame capture de ORVIX.
#include "orvix/capture/continuous_frame_capture.hpp"
// Incluye las declaraciones de diagnostic error de ORVIX.
#include "orvix/capture/diagnostic_error.hpp"
// Incluye las declaraciones de video format selector de ORVIX.
#include "orvix/capture/video_format_selector.hpp"

// Incluye fcntl.h para usar apertura y opciones de descriptores.
#include <fcntl.h>
// Incluye linux/videodev2.h para usar tipos y operaciones de V4L2.
#include <linux/videodev2.h>
// Incluye poll.h para usar espera sobre descriptores.
#include <poll.h>
// Incluye sys/ioctl.h para usar comandos de control de dispositivos.
#include <sys/ioctl.h>
// Incluye sys/mman.h para usar mapeo de memoria.
#include <sys/mman.h>
// Incluye unistd.h para usar operaciones POSIX.
#include <unistd.h>

// Incluye algorithm para usar algoritmos sobre rangos.
#include <algorithm>
// Incluye cerrno para usar códigos de error del sistema.
#include <cerrno>
// Incluye chrono para usar relojes y duraciones.
#include <chrono>
// Incluye cstddef para usar tipos de tamaño y bytes.
#include <cstddef>
// Incluye cstdint para usar enteros de tamaño fijo.
#include <cstdint>
// Incluye cstring para usar operaciones de memoria y cadenas de C.
#include <cstring>
// Incluye ratio para usar proporciones en tiempo de compilación.
#include <ratio>
// Incluye stdexcept para usar tipos estándar de excepciones.
#include <stdexcept>
// Incluye string para usar cadenas de texto.
#include <string>
// Incluye utility para usar utilidades de movimiento e intercambio.
#include <utility>
// Incluye vector para usar contenedores dinámicos.
#include <vector>

// Agrupa las declaraciones bajo orvix::capture.
namespace orvix::capture {

// Limita estas declaraciones a esta unidad de compilación.
namespace {

// Declara el tipo NativeFormat y las operaciones o datos de su contrato.
struct NativeFormat final {
    // Declara format (formato de video) con el tipo indicado.
    VideoFormat format;
    // Inicializa pixel_format (formato de píxeles) con inicialización por valor.
    std::uint32_t pixel_format{};
};

// Declara el tipo MappedBuffer y las operaciones o datos de su contrato.
struct MappedBuffer final {
    // Inicializa address con MAP_FAILED.
    void* address{MAP_FAILED};
    // Inicializa length con inicialización por valor.
    std::size_t length{};
};

// Define la operación retry_ioctl, que reintenta la llamada a V4L2 si la interrumpe una señal.
int retry_ioctl(const int descriptor, const unsigned long request, void* value) {
    // Establece result a partir de 0.
    int result = 0;

    // Ejecuta el cuerpo antes de evaluar la condición.
    do {
        // Establece result a partir de ::ioctl(descriptor, request, value).
        result = ::ioctl(descriptor, request, value);
    // Repite mientras se cumpla result < 0 && errno == EINTR.
    } while (result < 0 && errno == EINTR);

    // Devuelve result.
    return result;
}

// Declara el atributo de compilación [[noreturn]].
[[noreturn]]
// Define la operación throw_system_error con los parámetros declarados.
void throw_system_error(const std::string& operation) {
    // Interrumpe la operación con std::runtime_error(.
    throw std::runtime_error(
        // Ejecuta operation + " failed: " + std::string(std::strerror(errno)).
        operation + " failed: " + std::string(std::strerror(errno))
    );
}

// Define la operación pixel_format_name con los parámetros declarados.
std::string pixel_format_name(const std::uint32_t value) {
    // Selecciona una rama según value.
    switch (value) {
    // Selecciona el caso V4L2_PIX_FMT_NV12.
    case V4L2_PIX_FMT_NV12:
        // Devuelve "NV12".
        return "NV12";
    // Selecciona el caso V4L2_PIX_FMT_YUYV.
    case V4L2_PIX_FMT_YUYV:
        // Devuelve "YUY2".
        return "YUY2";
    // Selecciona el caso V4L2_PIX_FMT_UYVY.
    case V4L2_PIX_FMT_UYVY:
        // Devuelve "UYVY".
        return "UYVY";
    // Selecciona el caso V4L2_PIX_FMT_YUV420.
    case V4L2_PIX_FMT_YUV420:
        // Devuelve "I420".
        return "I420";
    // Selecciona el caso V4L2_PIX_FMT_RGB24.
    case V4L2_PIX_FMT_RGB24:
        // Devuelve "RGB24".
        return "RGB24";
    // Selecciona el caso V4L2_PIX_FMT_BGR24.
    case V4L2_PIX_FMT_BGR24:
        // Devuelve "BGR24".
        return "BGR24";
    // Selecciona el caso V4L2_PIX_FMT_MJPEG.
    case V4L2_PIX_FMT_MJPEG:
        // Devuelve "MJPG".
        return "MJPG";
    // Selecciona el caso V4L2_PIX_FMT_H264.
    case V4L2_PIX_FMT_H264:
        // Devuelve "H264".
        return "H264";
    // Atiende los valores que no coinciden con los casos anteriores.
    default: {
        // Ejecuta std::string fourcc(4, ' ').
        std::string fourcc(4, ' ');
        // Establece fourcc[0] a partir de static_cast<char>(value & 0xffU).
        fourcc[0] = static_cast<char>(value & 0xffU);
        // Establece fourcc[1] a partir de static_cast<char>((value >> 8U) & 0xffU).
        fourcc[1] = static_cast<char>((value >> 8U) & 0xffU);
        // Establece fourcc[2] a partir de static_cast<char>((value >> 16U) & 0xffU).
        fourcc[2] = static_cast<char>((value >> 16U) & 0xffU);
        // Establece fourcc[3] a partir de static_cast<char>((value >> 24U) & 0xffU).
        fourcc[3] = static_cast<char>((value >> 24U) & 0xffU);
        // Devuelve fourcc.
        return fourcc;
    }
    }
}

// Define la operación compressed_pixel_format con los parámetros declarados.
bool compressed_pixel_format(const std::uint32_t value) noexcept {
    // Devuelve el resultado de value == V4L2_PIX_FMT_MJPEG ||.
    return
        // Combina esta condición con value == V4L2_PIX_FMT_MJPEG ||.
        value == V4L2_PIX_FMT_MJPEG ||
        // Evalúa value == V4L2_PIX_FMT_H264 como parte de la condición.
        value == V4L2_PIX_FMT_H264;
}

// Define la operación compressed_format con los parámetros declarados.
bool compressed_format(
    // Declara el parámetro description.
    const v4l2_fmtdesc& description,
    // Incorpora const std::uint32_t pixel_format en esta operación.
    const std::uint32_t pixel_format
// Incorpora ) noexcept { en esta operación.
) noexcept {
    // Devuelve el resultado de (description.flags & V4L2_FMT_FLAG_COMPRESSED) != 0 ||.
    return
        // Combina esta condición con (description.flags & V4L2_FMT_FLAG_COMPRESSED) != 0 ||.
        (description.flags & V4L2_FMT_FLAG_COMPRESSED) != 0 ||
        // Combina esta condición con pixel_format == V4L2_PIX_FMT_MJPEG ||.
        pixel_format == V4L2_PIX_FMT_MJPEG ||
        // Evalúa pixel_format == V4L2_PIX_FMT_H264 como parte de la condición.
        pixel_format == V4L2_PIX_FMT_H264;
}

// Define la operación append_intervals con los parámetros declarados.
void append_intervals(
    // Declara el parámetro descriptor (descriptor del dispositivo).
    const int descriptor,
    // Declara el parámetro description.
    const v4l2_fmtdesc& description,
    // Declara el parámetro width (ancho en píxeles).
    const std::uint32_t width,
    // Declara el parámetro height (alto en píxeles).
    const std::uint32_t height,
    // Incorpora std::vector<NativeFormat>& formats en esta operación.
    std::vector<NativeFormat>& formats
) {
    // Establece interval_added a partir de false.
    bool interval_added = false;

    // Recorre según std::uint32_t index = 0;; ++index.
    for (std::uint32_t index = 0;; ++index) {
        // Inicializa interval con inicialización por valor.
        v4l2_frmivalenum interval{};
        // Establece interval.index a partir de index.
        interval.index = index;
        // Establece interval.pixel_format (formato de píxeles) a partir de description.pixelformat.
        interval.pixel_format = description.pixelformat;
        // Establece interval.width (ancho en píxeles) a partir de width.
        interval.width = width;
        // Establece interval.height (alto en píxeles) a partir de height.
        interval.height = height;

        // Evalúa la condición retry_ioctl(descriptor, VIDIOC_ENUM_FRAMEINTERVALS, &interval) < 0.
        if (retry_ioctl(descriptor, VIDIOC_ENUM_FRAMEINTERVALS, &interval) < 0) {
            // Evalúa la condición errno == EINVAL.
            if (errno == EINVAL) {
                // Termina el bucle o la rama actual.
                break;
            }
            // Interrumpe la operación con _system_error("VIDIOC_ENUM_FRAMEINTERVALS").
            throw_system_error("VIDIOC_ENUM_FRAMEINTERVALS");
        }

        // Establece numerator a partir de 30.
        std::uint32_t numerator = 30;
        // Establece denominator a partir de 1.
        std::uint32_t denominator = 1;

        // Evalúa la condición interval.type == V4L2_FRMIVAL_TYPE_DISCRETE.
        if (interval.type == V4L2_FRMIVAL_TYPE_DISCRETE) {
            // Establece numerator a partir de interval.discrete.denominator.
            numerator = interval.discrete.denominator;
            // Establece denominator a partir de interval.discrete.numerator.
            denominator = interval.discrete.numerator;
        }
        // Ejecuta la rama alternativa si la condición anterior fue falsa.
        else {
            // Establece minimum a partir de interval.stepwise.min.
            const auto& minimum = interval.stepwise.min;
            // Establece numerator a partir de minimum.denominator.
            numerator = minimum.denominator;
            // Establece denominator a partir de minimum.numerator.
            denominator = minimum.numerator;
        }

        // Evalúa la condición numerator == 0 || denominator == 0.
        if (numerator == 0 || denominator == 0) {
            // Pasa a la siguiente iteración.
            continue;
        }

        // Agrega un elemento al final del contenedor mediante formats.push_back.
        formats.push_back({
            {
                // Ejecuta formats.size(),.
                formats.size(),
                // Aporta width como elemento de la lista.
                width,
                // Aporta height como elemento de la lista.
                height,
                // Aporta numerator como elemento de la lista.
                numerator,
                // Aporta denominator como elemento de la lista.
                denominator,
                // Ejecuta pixel_format_name(description.pixelformat),.
                pixel_format_name(description.pixelformat),
                // Ejecuta compressed_format(description, description.pixelformat).
                compressed_format(description, description.pixelformat)
            },
            // Incorpora description.pixelformat en esta operación.
            description.pixelformat
        });
        // Establece interval_added a partir de true.
        interval_added = true;

        // Evalúa la condición interval.type != V4L2_FRMIVAL_TYPE_DISCRETE.
        if (interval.type != V4L2_FRMIVAL_TYPE_DISCRETE) {
            // Termina el bucle o la rama actual.
            break;
        }
    }

    // Evalúa la condición !interval_added.
    if (!interval_added) {
        // Agrega un elemento al final del contenedor mediante formats.push_back.
        formats.push_back({
            {
                // Ejecuta formats.size(),.
                formats.size(),
                // Aporta width como elemento de la lista.
                width,
                // Aporta height como elemento de la lista.
                height,
                // Aporta 30 como elemento de la lista.
                30,
                // Aporta 1 como elemento de la lista.
                1,
                // Ejecuta pixel_format_name(description.pixelformat),.
                pixel_format_name(description.pixelformat),
                // Ejecuta compressed_format(description, description.pixelformat).
                compressed_format(description, description.pixelformat)
            },
            // Incorpora description.pixelformat en esta operación.
            description.pixelformat
        });
    }
}

// Define la operación enumerate_formats con los parámetros declarados.
std::vector<NativeFormat> enumerate_formats(
    // Declara el parámetro descriptor (descriptor del dispositivo).
    const int descriptor,
    // Incorpora const VideoFormatTarget& target en esta operación.
    const VideoFormatTarget& target
) {
    // Declara formats con el tipo indicado.
    std::vector<NativeFormat> formats;

    // Recorre según std::uint32_t format_index = 0;; ++format_index.
    for (std::uint32_t format_index = 0;; ++format_index) {
        // Inicializa description con inicialización por valor.
        v4l2_fmtdesc description{};
        // Establece description.index a partir de format_index.
        description.index = format_index;
        // Establece description.type a partir de V4L2_BUF_TYPE_VIDEO_CAPTURE.
        description.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

        // Evalúa la condición retry_ioctl(descriptor, VIDIOC_ENUM_FMT, &description) < 0.
        if (retry_ioctl(descriptor, VIDIOC_ENUM_FMT, &description) < 0) {
            // Evalúa la condición errno == EINVAL.
            if (errno == EINVAL) {
                // Termina el bucle o la rama actual.
                break;
            }
            // Interrumpe la operación con _system_error("VIDIOC_ENUM_FMT").
            throw_system_error("VIDIOC_ENUM_FMT");
        }

        // Establece size_added a partir de false.
        bool size_added = false;

        // Recorre según std::uint32_t size_index = 0;; ++size_index.
        for (std::uint32_t size_index = 0;; ++size_index) {
            // Inicializa size con inicialización por valor.
            v4l2_frmsizeenum size{};
            // Establece size.index a partir de size_index.
            size.index = size_index;
            // Establece size.pixel_format (formato de píxeles) a partir de description.pixelformat.
            size.pixel_format = description.pixelformat;

            // Evalúa la condición retry_ioctl(descriptor, VIDIOC_ENUM_FRAMESIZES, &size) < 0.
            if (retry_ioctl(descriptor, VIDIOC_ENUM_FRAMESIZES, &size) < 0) {
                // Evalúa la condición errno == EINVAL.
                if (errno == EINVAL) {
                    // Termina el bucle o la rama actual.
                    break;
                }
                // Interrumpe la operación con _system_error("VIDIOC_ENUM_FRAMESIZES").
                throw_system_error("VIDIOC_ENUM_FRAMESIZES");
            }

            // Establece width (ancho en píxeles) a partir de target.width.
            std::uint32_t width = target.width;
            // Establece height (alto en píxeles) a partir de target.height.
            std::uint32_t height = target.height;

            // Evalúa la condición size.type == V4L2_FRMSIZE_TYPE_DISCRETE.
            if (size.type == V4L2_FRMSIZE_TYPE_DISCRETE) {
                // Establece width (ancho en píxeles) a partir de size.discrete.width.
                width = size.discrete.width;
                // Establece height (alto en píxeles) a partir de size.discrete.height.
                height = size.discrete.height;
            }
            // Ejecuta la rama alternativa si la condición anterior fue falsa.
            else {
                // Establece range a partir de size.stepwise.
                const auto& range = size.stepwise;
                // Establece width (ancho en píxeles) a partir de std::clamp(.
                width = std::clamp(
                    // Aporta target.width como elemento de la lista.
                    target.width,
                    // Aporta range.min_width como elemento de la lista.
                    range.min_width,
                    // Incorpora range.max_width en esta operación.
                    range.max_width
                );
                // Establece height (alto en píxeles) a partir de std::clamp(.
                height = std::clamp(
                    // Aporta target.height como elemento de la lista.
                    target.height,
                    // Aporta range.min_height como elemento de la lista.
                    range.min_height,
                    // Incorpora range.max_height en esta operación.
                    range.max_height
                );
            }

            // Ejecuta append_intervals(.
            append_intervals(
                // Aporta descriptor como elemento de la lista.
                descriptor,
                // Aporta description como elemento de la lista.
                description,
                // Aporta width como elemento de la lista.
                width,
                // Aporta height como elemento de la lista.
                height,
                // Incorpora formats en esta operación.
                formats
            );
            // Establece size_added a partir de true.
            size_added = true;

            // Evalúa la condición size.type != V4L2_FRMSIZE_TYPE_DISCRETE.
            if (size.type != V4L2_FRMSIZE_TYPE_DISCRETE) {
                // Termina el bucle o la rama actual.
                break;
            }
        }

        // Evalúa la condición !size_added.
        if (!size_added) {
            // Inicializa current con inicialización por valor.
            v4l2_format current{};
            // Establece current.type a partir de V4L2_BUF_TYPE_VIDEO_CAPTURE.
            current.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

            // Evalúa la condición retry_ioctl(descriptor, VIDIOC_G_FMT, &current) == 0.
            if (retry_ioctl(descriptor, VIDIOC_G_FMT, &current) == 0) {
                // Ejecuta append_intervals(.
                append_intervals(
                    // Aporta descriptor como elemento de la lista.
                    descriptor,
                    // Aporta description como elemento de la lista.
                    description,
                    // Aporta current.fmt.pix.width como elemento de la lista.
                    current.fmt.pix.width,
                    // Aporta current.fmt.pix.height como elemento de la lista.
                    current.fmt.pix.height,
                    // Incorpora formats en esta operación.
                    formats
                );
            }
        }
    }

    // Devuelve formats.
    return formats;
}

// Declara el tipo V4l2FrameReader y las operaciones o datos de su contrato.
class V4l2FrameReader final : public FrameReader {
// Expone los miembros de la interfaz.
public:
    // Ejecuta V4l2FrameReader(.
    V4l2FrameReader(
        // Declara el parámetro descriptor (descriptor del dispositivo).
        const int descriptor,
        // Declara el parámetro buffers.
        const std::vector<MappedBuffer>& buffers,
        // Incorpora const std::uint32_t stride en esta operación.
        const std::uint32_t stride
    )
        // Inicializa el miembro indicado por descriptor_(descriptor), antes del cuerpo.
        : descriptor_(descriptor),
          // Ejecuta buffers_(buffers),.
          buffers_(buffers),
          // Ejecuta stride_(stride) {}.
          stride_(stride) {}

    // Define la operación read_next con los parámetros declarados.
    FrameReadResult read_next() override {
        // Inicializa descriptor_status con inicialización por valor.
        pollfd descriptor_status{};
        // Establece descriptor_status.fd a partir de descriptor_.
        descriptor_status.fd = descriptor_;
        // Establece descriptor_status.events (eventos de diagnóstico) a partir de POLLIN | POLLPRI.
        descriptor_status.events = POLLIN | POLLPRI;

        // Establece poll_result a partir de ::poll(&descriptor_status, 1, 250).
        const int poll_result = ::poll(&descriptor_status, 1, 250);

        // Evalúa la condición poll_result == 0.
        if (poll_result == 0) {
            // Devuelve {}.
            return {};
        }

        // Evalúa la condición poll_result < 0.
        if (poll_result < 0) {
            // Evalúa la condición errno == EINTR.
            if (errno == EINTR) {
                // Devuelve {}.
                return {};
            }
            // Interrumpe la operación con _system_error("poll(camera)").
            throw_system_error("poll(camera)");
        }

        // Evalúa la condición (descriptor_status.revents & (POLLERR | POLLHUP | POLLNVAL)) != 0.
        if (
            // Evalúa (descriptor_status.revents & (POLLERR | POLLHUP | POLLNVAL)) != 0 como parte de la condición.
            (descriptor_status.revents & (POLLERR | POLLHUP | POLLNVAL)) != 0
        ) {
            // Interrumpe la operación con DiagnosticError(.
            throw DiagnosticError(
                // Aporta el texto "ORV-CAP-503" a la expresión.
                "ORV-CAP-503",
                // Aporta el texto "camera_disconnected" a la expresión.
                "camera_disconnected",
                // Aporta el texto "The V4L2 camera was disconnected during capture." a la expresión.
                "The V4L2 camera was disconnected during capture.",
                // Incorpora 74 en esta operación.
                74
            );
        }

        // Inicializa buffer (búfer de datos) con inicialización por valor.
        v4l2_buffer buffer{};
        // Establece buffer.type a partir de V4L2_BUF_TYPE_VIDEO_CAPTURE.
        buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        // Establece buffer.memory a partir de V4L2_MEMORY_MMAP.
        buffer.memory = V4L2_MEMORY_MMAP;

        // Evalúa la condición retry_ioctl(descriptor_, VIDIOC_DQBUF, &buffer) < 0.
        if (retry_ioctl(descriptor_, VIDIOC_DQBUF, &buffer) < 0) {
            // Evalúa la condición errno == EAGAIN.
            if (errno == EAGAIN) {
                // Devuelve {}.
                return {};
            }
            // Evalúa la condición errno == ENODEV || errno == EIO.
            if (errno == ENODEV || errno == EIO) {
                // Interrumpe la operación con DiagnosticError(.
                throw DiagnosticError(
                    // Aporta el texto "ORV-CAP-503" a la expresión.
                    "ORV-CAP-503",
                    // Aporta el texto "camera_disconnected" a la expresión.
                    "camera_disconnected",
                    // Aporta el texto "The V4L2 camera stopped responding during capture." a la expresión.
                    "The V4L2 camera stopped responding during capture.",
                    // Incorpora 74 en esta operación.
                    74
                );
            }
            // Interrumpe la operación con _system_error("VIDIOC_DQBUF").
            throw_system_error("VIDIOC_DQBUF");
        }

        // Establece timestamp (marca de tiempo) a partir de static_cast<std::int64_t>(buffer.timestamp.tv_sec) * 10'000'000LL +.
        std::int64_t timestamp =
            // Incorpora static_cast<std::int64_t>(buffer.timestamp.tv_sec) * 10'000'000LL + en esta operación.
            static_cast<std::int64_t>(buffer.timestamp.tv_sec) * 10'000'000LL +
            // Incorpora static_cast<std::int64_t>(buffer.timestamp.tv_usec) * 10LL en esta operación.
            static_cast<std::int64_t>(buffer.timestamp.tv_usec) * 10LL;

        // Evalúa la condición timestamp <= 0.
        if (timestamp <= 0) {
            // Establece timestamp (marca de tiempo) a partir de std::chrono::duration_cast<.
            timestamp = std::chrono::duration_cast<
                // Incorpora std::chrono::duration<std::int64_t, std::ratio<1, 10'000'000>> en esta operación.
                std::chrono::duration<std::int64_t, std::ratio<1, 10'000'000>>
            // Ejecuta >(std::chrono::steady_clock::now().time_since_epoch()).count().
            >(std::chrono::steady_clock::now().time_since_epoch()).count();
        }
        // Establece byte_count a partir de buffer.bytesused.
        const std::size_t byte_count = buffer.bytesused;

        // Evalúa la condición buffer.index >= buffers_.size() ||.
        if (
            // Combina esta condición con buffer.index >= buffers_.size() ||.
            buffer.index >= buffers_.size() ||
            // Evalúa byte_count > buffers_[buffer.index].length como parte de la condición.
            byte_count > buffers_[buffer.index].length
        ) {
            // Interrumpe la operación con std::runtime_error(.
            throw std::runtime_error(
                // Aporta el texto "V4L2 returned an invalid capture buffer." a la expresión.
                "V4L2 returned an invalid capture buffer."
            );
        }

        // Ejecuta std::vector<std::byte> payload(byte_count).
        std::vector<std::byte> payload(byte_count);
        // Evalúa la condición byte_count != 0.
        if (byte_count != 0) {
            // Copia bytes entre dos regiones de memoria mediante std::memcpy.
            std::memcpy(
                // Ejecuta payload.data(),.
                payload.data(),
                // Aporta buffers_[buffer.index].address como elemento de la lista.
                buffers_[buffer.index].address,
                // Incorpora byte_count en esta operación.
                byte_count
            );
        }

        // Evalúa la condición retry_ioctl(descriptor_, VIDIOC_QBUF, &buffer) < 0.
        if (retry_ioctl(descriptor_, VIDIOC_QBUF, &buffer) < 0) {
            // Evalúa la condición errno == ENODEV || errno == EIO.
            if (errno == ENODEV || errno == EIO) {
                // Interrumpe la operación con DiagnosticError(.
                throw DiagnosticError(
                    // Aporta el texto "ORV-CAP-503" a la expresión.
                    "ORV-CAP-503",
                    // Aporta el texto "camera_disconnected" a la expresión.
                    "camera_disconnected",
                    // Aporta el texto "The V4L2 camera was removed while returning a frame." a la expresión.
                    "The V4L2 camera was removed while returning a frame.",
                    // Incorpora 74 en esta operación.
                    74
                );
            }
            // Interrumpe la operación con _system_error("VIDIOC_QBUF").
            throw_system_error("VIDIOC_QBUF");
        }

        // Devuelve {.
        return {
            // Aporta true como elemento de la lista.
            true,
            // Aporta timestamp como elemento de la lista.
            timestamp,
            // Aporta byte_count como elemento de la lista.
            byte_count,
            // Aporta stride_ como elemento de la lista.
            stride_,
            // Transfiere el valor sin exigir una copia mediante std::move.
            std::move(payload)
        };
    }

// Restringe estos miembros a la implementación.
private:
    // Declara descriptor_ (descriptor del dispositivo) con el tipo indicado.
    int descriptor_;
    // Declara buffers_ con el tipo indicado.
    const std::vector<MappedBuffer>& buffers_;
    // Inicializa stride_ (bytes entre filas de imagen) con inicialización por valor.
    std::uint32_t stride_{};
};

// Incorpora } // namespace en esta operación.
}  // namespace

// Declara el tipo V4l2Camera y las operaciones o datos de su contrato.
struct V4l2Camera::Impl final {
    // Inicializa descriptor (descriptor del dispositivo) con -1.
    int descriptor{-1};
    // Declara buffers con el tipo indicado.
    std::vector<MappedBuffer> buffers;
    // Declara configured_format con el tipo indicado.
    VideoFormat configured_format;
    // Inicializa streaming con false.
    bool streaming{false};
    // Inicializa format_configured con false.
    bool format_configured{false};
};

// Ejecuta V4l2Camera::V4l2Camera().
V4l2Camera::V4l2Camera()
    // Inicializa el miembro indicado por impl_(std::make_unique<Impl>()) {} antes del cuerpo.
    : impl_(std::make_unique<Impl>()) {}

// Ejecuta V4l2Camera::~V4l2Camera() {.
V4l2Camera::~V4l2Camera() {
    // Cierra el recurso mediante close.
    close();
}

// Define la operación V4l2Camera::open con los parámetros declarados.
void V4l2Camera::open(const CameraDevice& device) {
    // Evalúa la condición is_open().
    if (is_open()) {
        // Interrumpe la operación con std::logic_error("A camera is already open.").
        throw std::logic_error("A camera is already open.");
    }

    // Evalúa la condición device.symbolic_link.empty().
    if (device.symbolic_link.empty()) {
        // Interrumpe la operación con std::invalid_argument("The selected camera path is empty.").
        throw std::invalid_argument("The selected camera path is empty.");
    }

    // Establece descriptor (descriptor del dispositivo) a partir de ::open(.
    const int descriptor = ::open(
        // Ejecuta device.symbolic_link.c_str(),.
        device.symbolic_link.c_str(),
        // Incorpora O_RDWR | O_NONBLOCK en esta operación.
        O_RDWR | O_NONBLOCK
    );

    // Evalúa la condición descriptor < 0.
    if (descriptor < 0) {
        // Interrumpe la operación con _system_error("open(" + device.symbolic_link + ")").
        throw_system_error("open(" + device.symbolic_link + ")");
    }

    // Inicializa capability con inicialización por valor.
    v4l2_capability capability{};
    // Evalúa la condición retry_ioctl(descriptor, VIDIOC_QUERYCAP, &capability) < 0.
    if (retry_ioctl(descriptor, VIDIOC_QUERYCAP, &capability) < 0) {
        // Establece saved_error a partir de errno.
        const int saved_error = errno;
        // Cierra el recurso mediante close.
        static_cast<void>(::close(descriptor));
        // Establece errno a partir de saved_error.
        errno = saved_error;
        // Interrumpe la operación con _system_error("VIDIOC_QUERYCAP").
        throw_system_error("VIDIOC_QUERYCAP");
    }

    // Establece capabilities a partir de (capability.capabilities & V4L2_CAP_DEVICE_CAPS) != 0.
    const std::uint32_t capabilities =
        // Evalúa (capability.capabilities & V4L2_CAP_DEVICE_CAPS) != 0 como parte de la condición.
        (capability.capabilities & V4L2_CAP_DEVICE_CAPS) != 0
            // Incorpora ? capability.device_caps en esta operación.
            ? capability.device_caps
            // Inicializa el miembro indicado por capability.capabilities antes del cuerpo.
            : capability.capabilities;

    // Evalúa la condición (capabilities & V4L2_CAP_VIDEO_CAPTURE) == 0 ||.
    if (
        // Combina esta condición con (capabilities & V4L2_CAP_VIDEO_CAPTURE) == 0 ||.
        (capabilities & V4L2_CAP_VIDEO_CAPTURE) == 0 ||
        // Evalúa (capabilities & V4L2_CAP_STREAMING) == 0 como parte de la condición.
        (capabilities & V4L2_CAP_STREAMING) == 0
    ) {
        // Cierra el recurso mediante close.
        static_cast<void>(::close(descriptor));
        // Interrumpe la operación con std::runtime_error(.
        throw std::runtime_error(
            // Aporta el texto "The selected V4L2 device does not support streaming capture." a la expresión.
            "The selected V4L2 device does not support streaming capture."
        );
    }

    // Establece impl_->descriptor a partir de descriptor.
    impl_->descriptor = descriptor;
}

// Define la operación V4l2Camera::close con los parámetros declarados.
void V4l2Camera::close() noexcept {
    // Evalúa la condición impl_ == nullptr.
    if (impl_ == nullptr) {
        // Termina la función sin un valor de retorno.
        return;
    }

    // Evalúa la condición impl_->streaming && impl_->descriptor >= 0.
    if (impl_->streaming && impl_->descriptor >= 0) {
        // Establece type a partir de V4L2_BUF_TYPE_VIDEO_CAPTURE.
        v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        // Reintenta la llamada a V4L2 si la interrumpe una señal mediante retry_ioctl.
        static_cast<void>(retry_ioctl(
            // Aporta impl_->descriptor como elemento de la lista.
            impl_->descriptor,
            // Aporta VIDIOC_STREAMOFF como elemento de la lista.
            VIDIOC_STREAMOFF,
            // Incorpora &type en esta operación.
            &type
        ));
    }

    // Establece impl_->streaming a partir de false.
    impl_->streaming = false;

    // Recorre según const auto& buffer : impl_->buffers.
    for (const auto& buffer : impl_->buffers) {
        // Evalúa la condición buffer.address != MAP_FAILED.
        if (buffer.address != MAP_FAILED) {
            // Libera la región mapeada mediante munmap.
            static_cast<void>(::munmap(buffer.address, buffer.length));
        }
    }

    // Vacía el contenedor mediante impl_->buffers.clear.
    impl_->buffers.clear();
    // Establece impl_->format_configured a partir de false.
    impl_->format_configured = false;
    // Establece impl_->configured_format a partir de {}.
    impl_->configured_format = {};

    // Evalúa la condición impl_->descriptor >= 0.
    if (impl_->descriptor >= 0) {
        // Cierra el recurso mediante close.
        static_cast<void>(::close(impl_->descriptor));
        // Establece impl_->descriptor a partir de -1.
        impl_->descriptor = -1;
    }
}

// Define la operación V4l2Camera::available_formats con los parámetros declarados.
std::vector<VideoFormat> V4l2Camera::available_formats() const {
    // Evalúa la condición !is_open().
    if (!is_open()) {
        // Interrumpe la operación con std::logic_error(.
        throw std::logic_error(
            // Aporta el texto "A camera must be open before enumerating video formats." a la expresión.
            "A camera must be open before enumerating video formats."
        );
    }

    // Establece native_formats a partir de enumerate_formats(.
    const auto native_formats = enumerate_formats(
        // Aporta impl_->descriptor como elemento de la lista.
        impl_->descriptor,
        // Incorpora VideoFormatTarget{} en esta operación.
        VideoFormatTarget{}
    );
    // Declara formats con el tipo indicado.
    std::vector<VideoFormat> formats;
    // Reserva capacidad en el contenedor mediante formats.reserve.
    formats.reserve(native_formats.size());

    // Recorre según const auto& native_format : native_formats.
    for (const auto& native_format : native_formats) {
        // Agrega un elemento al final del contenedor mediante formats.push_back.
        formats.push_back(native_format.format);
    }

    // Devuelve formats.
    return formats;
}

// Define la operación V4l2Camera::configure con los parámetros declarados.
VideoFormat V4l2Camera::configure(const VideoFormatTarget& target) {
    // Evalúa la condición !is_open().
    if (!is_open()) {
        // Interrumpe la operación con std::logic_error(.
        throw std::logic_error(
            // Aporta el texto "A camera must be open before configuring a video format." a la expresión.
            "A camera must be open before configuring a video format."
        );
    }

    // Establece native_formats a partir de enumerate_formats(impl_->descriptor, target).
    const auto native_formats = enumerate_formats(impl_->descriptor, target);
    // Declara formats con el tipo indicado.
    std::vector<VideoFormat> formats;
    // Reserva capacidad en el contenedor mediante formats.reserve.
    formats.reserve(native_formats.size());

    // Recorre según const auto& native_format : native_formats.
    for (const auto& native_format : native_formats) {
        // Agrega un elemento al final del contenedor mediante formats.push_back.
        formats.push_back(native_format.format);
    }

    // Establece selected a partir de VideoFormatSelector::select_best(.
    const VideoFormat& selected = VideoFormatSelector::select_best(
        // Aporta formats como elemento de la lista.
        formats,
        // Incorpora target en esta operación.
        target
    );
    // Establece native a partir de native_formats.at(.
    const NativeFormat& native = native_formats.at(
        // Incorpora selected.native_type_index en esta operación.
        selected.native_type_index
    );

    // Inicializa requested con inicialización por valor.
    v4l2_format requested{};
    // Establece requested.type a partir de V4L2_BUF_TYPE_VIDEO_CAPTURE.
    requested.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    // Establece requested.fmt.pix.width (ancho en píxeles) a partir de selected.width.
    requested.fmt.pix.width = selected.width;
    // Establece requested.fmt.pix.height (alto en píxeles) a partir de selected.height.
    requested.fmt.pix.height = selected.height;
    // Establece requested.fmt.pix.pixelformat a partir de native.pixel_format.
    requested.fmt.pix.pixelformat = native.pixel_format;
    // Establece requested.fmt.pix.field a partir de V4L2_FIELD_ANY.
    requested.fmt.pix.field = V4L2_FIELD_ANY;

    // Evalúa la condición retry_ioctl(impl_->descriptor, VIDIOC_S_FMT, &requested) < 0.
    if (retry_ioctl(impl_->descriptor, VIDIOC_S_FMT, &requested) < 0) {
        // Interrumpe la operación con _system_error("VIDIOC_S_FMT").
        throw_system_error("VIDIOC_S_FMT");
    }

    // Inicializa parameters con inicialización por valor.
    v4l2_streamparm parameters{};
    // Establece parameters.type a partir de V4L2_BUF_TYPE_VIDEO_CAPTURE.
    parameters.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    // Establece parameters.parm.capture.timeperframe.numerator a partir de selected.frame_rate_denominator.
    parameters.parm.capture.timeperframe.numerator =
        // Incorpora selected.frame_rate_denominator en esta operación.
        selected.frame_rate_denominator;
    // Establece parameters.parm.capture.timeperframe.denominator a partir de selected.frame_rate_numerator.
    parameters.parm.capture.timeperframe.denominator =
        // Incorpora selected.frame_rate_numerator en esta operación.
        selected.frame_rate_numerator;

    // Establece parameter_result a partir de retry_ioctl(.
    const int parameter_result = retry_ioctl(
        // Aporta impl_->descriptor como elemento de la lista.
        impl_->descriptor,
        // Aporta VIDIOC_S_PARM como elemento de la lista.
        VIDIOC_S_PARM,
        // Incorpora &parameters en esta operación.
        &parameters
    );

    // Evalúa la condición parameter_result < 0 && errno != EINVAL.
    if (parameter_result < 0 && errno != EINVAL) {
        // Interrumpe la operación con _system_error("VIDIOC_S_PARM").
        throw_system_error("VIDIOC_S_PARM");
    }

    // Establece configured a partir de selected.
    VideoFormat configured = selected;
    // Establece configured.width (ancho en píxeles) a partir de requested.fmt.pix.width.
    configured.width = requested.fmt.pix.width;
    // Establece configured.height (alto en píxeles) a partir de requested.fmt.pix.height.
    configured.height = requested.fmt.pix.height;
    // Establece configured.pixel_format (formato de píxeles) a partir de pixel_format_name(requested.fmt.pix.pixelformat).
    configured.pixel_format = pixel_format_name(requested.fmt.pix.pixelformat);
    // Establece configured.compressed a partir de compressed_pixel_format(.
    configured.compressed = compressed_pixel_format(
        // Incorpora requested.fmt.pix.pixelformat en esta operación.
        requested.fmt.pix.pixelformat
    );
    // Establece configured.stride (bytes entre filas de imagen) a partir de requested.fmt.pix.bytesperline.
    configured.stride = requested.fmt.pix.bytesperline;

    // Evalúa la condición parameter_result == 0 &&.
    if (
        // Combina esta condición con parameter_result == 0 &&.
        parameter_result == 0 &&
        // Combina esta condición con parameters.parm.capture.timeperframe.numerator != 0 &&.
        parameters.parm.capture.timeperframe.numerator != 0 &&
        // Evalúa parameters.parm.capture.timeperframe.denominator != 0 como parte de la condición.
        parameters.parm.capture.timeperframe.denominator != 0
    ) {
        // Establece configured.frame_rate_numerator a partir de parameters.parm.capture.timeperframe.denominator.
        configured.frame_rate_numerator =
            // Incorpora parameters.parm.capture.timeperframe.denominator en esta operación.
            parameters.parm.capture.timeperframe.denominator;
        // Establece configured.frame_rate_denominator a partir de parameters.parm.capture.timeperframe.numerator.
        configured.frame_rate_denominator =
            // Incorpora parameters.parm.capture.timeperframe.numerator en esta operación.
            parameters.parm.capture.timeperframe.numerator;
    }

    // Inicializa request con inicialización por valor.
    v4l2_requestbuffers request{};
    // Establece request.count a partir de 4.
    request.count = 4;
    // Establece request.type a partir de V4L2_BUF_TYPE_VIDEO_CAPTURE.
    request.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    // Establece request.memory a partir de V4L2_MEMORY_MMAP.
    request.memory = V4L2_MEMORY_MMAP;

    // Evalúa la condición retry_ioctl(impl_->descriptor, VIDIOC_REQBUFS, &request) < 0.
    if (retry_ioctl(impl_->descriptor, VIDIOC_REQBUFS, &request) < 0) {
        // Interrumpe la operación con _system_error("VIDIOC_REQBUFS").
        throw_system_error("VIDIOC_REQBUFS");
    }

    // Evalúa la condición request.count < 2.
    if (request.count < 2) {
        // Interrumpe la operación con std::runtime_error(.
        throw std::runtime_error(
            // Aporta el texto "The V4L2 camera did not provide enough capture buffers." a la expresión.
            "The V4L2 camera did not provide enough capture buffers."
        );
    }

    // Reserva capacidad en el contenedor mediante impl_->buffers.reserve.
    impl_->buffers.reserve(request.count);

    // Ejecuta el bloque protegido contra excepciones.
    try {
        // Recorre según std::uint32_t index = 0; index < request.count; ++index.
        for (std::uint32_t index = 0; index < request.count; ++index) {
            // Inicializa buffer (búfer de datos) con inicialización por valor.
            v4l2_buffer buffer{};
            // Establece buffer.type a partir de V4L2_BUF_TYPE_VIDEO_CAPTURE.
            buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            // Establece buffer.memory a partir de V4L2_MEMORY_MMAP.
            buffer.memory = V4L2_MEMORY_MMAP;
            // Establece buffer.index a partir de index.
            buffer.index = index;

            // Evalúa la condición retry_ioctl(impl_->descriptor, VIDIOC_QUERYBUF, &buffer) < 0.
            if (retry_ioctl(impl_->descriptor, VIDIOC_QUERYBUF, &buffer) < 0) {
                // Interrumpe la operación con _system_error("VIDIOC_QUERYBUF").
                throw_system_error("VIDIOC_QUERYBUF");
            }

            // Establece address a partir de ::mmap(.
            void* address = ::mmap(
                // Aporta nullptr como elemento de la lista.
                nullptr,
                // Aporta buffer.length como elemento de la lista.
                buffer.length,
                // Aporta PROT_READ | PROT_WRITE como elemento de la lista.
                PROT_READ | PROT_WRITE,
                // Aporta MAP_SHARED como elemento de la lista.
                MAP_SHARED,
                // Aporta impl_->descriptor como elemento de la lista.
                impl_->descriptor,
                // Incorpora static_cast<off_t>(buffer.m.offset) en esta operación.
                static_cast<off_t>(buffer.m.offset)
            );

            // Evalúa la condición address == MAP_FAILED.
            if (address == MAP_FAILED) {
                // Interrumpe la operación con _system_error("mmap(camera buffer)").
                throw_system_error("mmap(camera buffer)");
            }

            // Agrega un elemento al final del contenedor mediante impl_->buffers.push_back.
            impl_->buffers.push_back({address, buffer.length});

            // Evalúa la condición retry_ioctl(impl_->descriptor, VIDIOC_QBUF, &buffer) < 0.
            if (retry_ioctl(impl_->descriptor, VIDIOC_QBUF, &buffer) < 0) {
                // Interrumpe la operación con _system_error("VIDIOC_QBUF").
                throw_system_error("VIDIOC_QBUF");
            }
        }

        // Establece type a partir de V4L2_BUF_TYPE_VIDEO_CAPTURE.
        v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        // Evalúa la condición retry_ioctl(impl_->descriptor, VIDIOC_STREAMON, &type) < 0.
        if (retry_ioctl(impl_->descriptor, VIDIOC_STREAMON, &type) < 0) {
            // Interrumpe la operación con _system_error("VIDIOC_STREAMON").
            throw_system_error("VIDIOC_STREAMON");
        }
    }
    // Maneja la excepción declarada en catch (...) {.
    catch (...) {
        // Cierra el recurso mediante close.
        close();
        // Vuelve a propagar la excepción.
        throw;
    }

    // Establece impl_->streaming a partir de true.
    impl_->streaming = true;
    // Establece impl_->format_configured a partir de true.
    impl_->format_configured = true;
    // Establece impl_->configured_format a partir de configured.
    impl_->configured_format = configured;
    // Devuelve configured.
    return configured;
}

// Define la operación V4l2Camera::capture_frames con los parámetros declarados.
CaptureSummary V4l2Camera::capture_frames(
    // Declara el parámetro requested_frames.
    const std::size_t requested_frames,
    // Incorpora FrameSink* const sink en esta operación.
    FrameSink* const sink
) {
    // Evalúa la condición !is_open().
    if (!is_open()) {
        // Interrumpe la operación con std::logic_error("A camera must be open before capturing frames.").
        throw std::logic_error("A camera must be open before capturing frames.");
    }

    // Evalúa la condición !impl_->format_configured || !impl_->streaming.
    if (!impl_->format_configured || !impl_->streaming) {
        // Interrumpe la operación con std::logic_error(.
        throw std::logic_error(
            // Aporta el texto "A video format must be configured before capturing frames." a la expresión.
            "A video format must be configured before capturing frames."
        );
    }

    // Define la operación reader con los parámetros declarados.
    V4l2FrameReader reader(
        // Aporta impl_->descriptor como elemento de la lista.
        impl_->descriptor,
        // Aporta impl_->buffers como elemento de la lista.
        impl_->buffers,
        // Incorpora impl_->configured_format.stride en esta operación.
        impl_->configured_format.stride
    );
    // Devuelve ContinuousFrameCapture::run(.
    return ContinuousFrameCapture::run(
        // Aporta reader como elemento de la lista.
        reader,
        // Aporta requested_frames como elemento de la lista.
        requested_frames,
        // Aporta impl_->configured_format como elemento de la lista.
        impl_->configured_format,
        // Aporta 20 como elemento de la lista.
        20,
        // Incorpora sink en esta operación.
        sink
    );
}

// Define la operación V4l2Camera::is_open con los parámetros declarados.
bool V4l2Camera::is_open() const noexcept {
    // Devuelve impl_ != nullptr && impl_->descriptor >= 0.
    return impl_ != nullptr && impl_->descriptor >= 0;
}

// Incorpora } // namespace orvix::capture en esta operación.
}  // namespace orvix::capture
