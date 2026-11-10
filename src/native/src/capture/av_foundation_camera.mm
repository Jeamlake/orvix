// Incluye las declaraciones de av foundation camera de ORVIX.
#include "orvix/capture/av_foundation_camera.hpp"

// Incluye las declaraciones de continuous frame capture de ORVIX.
#include "orvix/capture/continuous_frame_capture.hpp"
// Incluye las declaraciones de diagnostic error de ORVIX.
#include "orvix/capture/diagnostic_error.hpp"
// Incluye las declaraciones de video format selector de ORVIX.
#include "orvix/capture/video_format_selector.hpp"

// Importa las declaraciones de AVFoundation/AVFoundation.h para esta unidad.
#import <AVFoundation/AVFoundation.h>
// Importa las declaraciones de CoreMedia/CoreMedia.h para esta unidad.
#import <CoreMedia/CoreMedia.h>
// Importa las declaraciones de CoreVideo/CoreVideo.h para esta unidad.
#import <CoreVideo/CoreVideo.h>
// Importa las declaraciones de Foundation/Foundation.h para esta unidad.
#import <Foundation/Foundation.h>

// Incluye algorithm para usar algoritmos sobre rangos.
#include <algorithm>
// Incluye chrono para usar relojes y duraciones.
#include <chrono>
// Incluye cmath para usar funciones matemáticas.
#include <cmath>
// Incluye condition_variable para usar espera coordinada entre hilos.
#include <condition_variable>
// Incluye cstddef para usar tipos de tamaño y bytes.
#include <cstddef>
// Incluye cstdint para usar enteros de tamaño fijo.
#include <cstdint>
// Incluye deque para usar colas de doble extremo.
#include <deque>
// Incluye cstring para usar operaciones de memoria y cadenas de C.
#include <cstring>
// Incluye limits para usar límites de tipos numéricos.
#include <limits>
// Incluye memory para usar punteros inteligentes y gestión de memoria.
#include <memory>
// Incluye mutex para usar exclusión mutua.
#include <mutex>
// Incluye numeric para usar algoritmos numéricos.
#include <numeric>
// Incluye stdexcept para usar tipos estándar de excepciones.
#include <stdexcept>
// Incluye string para usar cadenas de texto.
#include <string>
// Incluye utility para usar utilidades de movimiento e intercambio.
#include <utility>
// Incluye vector para usar contenedores dinámicos.
#include <vector>

// Agrupa las declaraciones bajo orvix::capture::detail.
namespace orvix::capture::detail {

// Declara el tipo SampleQueue y las operaciones o datos de su contrato.
struct SampleQueue final {
    // Declara mutex (mutex que protege el acceso compartido) con el tipo indicado.
    std::mutex mutex;
    // Declara ready con el tipo indicado.
    std::condition_variable ready;
    // Declara frames (fotogramas) con el tipo indicado.
    std::deque<orvix::capture::FrameReadResult> frames;
    // Inicializa callback_failed con false.
    bool callback_failed{false};
};

// Declara el tipo PixelBufferLock y las operaciones o datos de su contrato.
class PixelBufferLock final {
// Expone los miembros de la interfaz.
public:
    // Ejecuta explicit PixelBufferLock(CVPixelBufferRef buffer).
    explicit PixelBufferLock(CVPixelBufferRef buffer)
        // Inicializa el miembro indicado por buffer_(buffer) { antes del cuerpo.
        : buffer_(buffer) {
        // Establece result a partir de CVPixelBufferLockBaseAddress(.
        const CVReturn result = CVPixelBufferLockBaseAddress(
            // Aporta buffer_ como elemento de la lista.
            buffer_,
            // Incorpora kCVPixelBufferLock_ReadOnly en esta operación.
            kCVPixelBufferLock_ReadOnly
        );
        // Evalúa la condición result != kCVReturnSuccess.
        if (result != kCVReturnSuccess) {
            // Interrumpe la operación con std::runtime_error("Unable to lock an AVFoundation frame.").
            throw std::runtime_error("Unable to lock an AVFoundation frame.");
        }
    }

    // Define ~PixelBufferLock para cerrar los recursos al destruir la instancia.
    ~PixelBufferLock() {
        // Libera el bloqueo del búfer de píxeles mediante CVPixelBufferUnlockBaseAddress.
        CVPixelBufferUnlockBaseAddress(
            // Aporta buffer_ como elemento de la lista.
            buffer_,
            // Incorpora kCVPixelBufferLock_ReadOnly en esta operación.
            kCVPixelBufferLock_ReadOnly
        );
    }

    // Prohíbe esta operación en tiempo de compilación.
    PixelBufferLock(const PixelBufferLock&) = delete;
    // Establece operator a partir de (const PixelBufferLock&) = delete.
    PixelBufferLock& operator=(const PixelBufferLock&) = delete;

// Restringe estos miembros a la implementación.
private:
    // Declara buffer_ (búfer de datos) con el tipo indicado.
    CVPixelBufferRef buffer_;
};

// Define la operación checked_product, que multiplica tamaños comprobando desbordamiento.
std::size_t checked_product(
    // Declara el parámetro left.
    const std::size_t left,
    // Incorpora const std::size_t right en esta operación.
    const std::size_t right
) {
    // Evalúa la condición right != 0 &&.
    if (
        // Combina esta condición con right != 0 &&.
        right != 0 &&
        // Ejecuta left > std::numeric_limits<std::size_t>::max() / right.
        left > std::numeric_limits<std::size_t>::max() / right
    ) {
        // Interrumpe la operación con std::overflow_error("AVFoundation frame size overflowed.").
        throw std::overflow_error("AVFoundation frame size overflowed.");
    }
    // Devuelve left * right.
    return left * right;
}

// Define la operación frame_from_sample con los parámetros declarados.
orvix::capture::FrameReadResult frame_from_sample(
    // Declara el parámetro sample_buffer.
    CMSampleBufferRef sample_buffer,
    // Incorpora const std::int64_t timestamp_100ns en esta operación.
    const std::int64_t timestamp_100ns
) {
    // Establece pixel_buffer a partir de CMSampleBufferGetImageBuffer(sample_buffer).
    CVPixelBufferRef pixel_buffer =
        // Extrae el búfer de imagen de la muestra mediante CMSampleBufferGetImageBuffer.
        CMSampleBufferGetImageBuffer(sample_buffer);
    // Evalúa la condición pixel_buffer == nullptr.
    if (pixel_buffer == nullptr) {
        // Interrumpe la operación con std::runtime_error(.
        throw std::runtime_error(
            // Aporta el texto "AVFoundation did not provide a pixel buffer." a la expresión.
            "AVFoundation did not provide a pixel buffer."
        );
    }

    // Ejecuta PixelBufferLock lock(pixel_buffer).
    PixelBufferLock lock(pixel_buffer);
    // Establece width (ancho en píxeles) a partir de CVPixelBufferGetWidth(pixel_buffer).
    const std::size_t width = CVPixelBufferGetWidth(pixel_buffer);
    // Establece height (alto en píxeles) a partir de CVPixelBufferGetHeight(pixel_buffer).
    const std::size_t height = CVPixelBufferGetHeight(pixel_buffer);
    // Establece format (formato de video) a partir de CVPixelBufferGetPixelFormatType(pixel_buffer).
    const FourCharCode format = CVPixelBufferGetPixelFormatType(pixel_buffer);
    // Establece stride (bytes entre filas de imagen) a partir de 0.
    std::uint32_t stride = 0;
    // Declara payload (carga útil del fotograma) con el tipo indicado.
    std::vector<std::byte> payload;

    // Evalúa la condición CVPixelBufferIsPlanar(pixel_buffer).
    if (CVPixelBufferIsPlanar(pixel_buffer)) {
        // Establece plane_count a partir de CVPixelBufferGetPlaneCount(pixel_buffer).
        const std::size_t plane_count =
            // Ejecuta CVPixelBufferGetPlaneCount(pixel_buffer).
            CVPixelBufferGetPlaneCount(pixel_buffer);
        // Evalúa la condición plane_count == 0.
        if (plane_count == 0) {
            // Interrumpe la operación con std::runtime_error(.
            throw std::runtime_error(
                // Aporta el texto "AVFoundation returned an empty planar pixel buffer." a la expresión.
                "AVFoundation returned an empty planar pixel buffer."
            );
        }

        // Establece stride (bytes entre filas de imagen) a partir de static_cast<std::uint32_t>(width).
        stride = static_cast<std::uint32_t>(width);
        // Establece total_size a partir de 0.
        std::size_t total_size = 0;
        // Recorre según std::size_t plane = 0; plane < plane_count; ++plane.
        for (std::size_t plane = 0; plane < plane_count; ++plane) {
            // Establece plane_height a partir de CVPixelBufferGetHeightOfPlane(pixel_buffer, plane).
            const std::size_t plane_height =
                // Ejecuta CVPixelBufferGetHeightOfPlane(pixel_buffer, plane).
                CVPixelBufferGetHeightOfPlane(pixel_buffer, plane);
            // Actualiza total_size con += y checked_product(width, plane_height).
            total_size += checked_product(width, plane_height);
        }
        // Ajusta el tamaño del contenedor mediante payload.resize.
        payload.resize(total_size);

        // Establece destination_offset a partir de 0.
        std::size_t destination_offset = 0;
        // Recorre según std::size_t plane = 0; plane < plane_count; ++plane.
        for (std::size_t plane = 0; plane < plane_count; ++plane) {
            // Establece source (fuente de video) a partir de static_cast<const std::byte*>(.
            const auto* source = static_cast<const std::byte*>(
                // Ejecuta CVPixelBufferGetBaseAddressOfPlane(pixel_buffer, plane).
                CVPixelBufferGetBaseAddressOfPlane(pixel_buffer, plane)
            );
            // Establece source_stride a partir de CVPixelBufferGetBytesPerRowOfPlane(pixel_buffer, plane).
            const std::size_t source_stride =
                // Ejecuta CVPixelBufferGetBytesPerRowOfPlane(pixel_buffer, plane).
                CVPixelBufferGetBytesPerRowOfPlane(pixel_buffer, plane);
            // Establece plane_height a partir de CVPixelBufferGetHeightOfPlane(pixel_buffer, plane).
            const std::size_t plane_height =
                // Ejecuta CVPixelBufferGetHeightOfPlane(pixel_buffer, plane).
                CVPixelBufferGetHeightOfPlane(pixel_buffer, plane);
            // Evalúa la condición source == nullptr || source_stride < width.
            if (source == nullptr || source_stride < width) {
                // Interrumpe la operación con std::runtime_error(.
                throw std::runtime_error(
                    // Aporta el texto "AVFoundation returned an invalid pixel-buffer plane." a la expresión.
                    "AVFoundation returned an invalid pixel-buffer plane."
                );
            }
            // Recorre según std::size_t row = 0; row < plane_height; ++row.
            for (std::size_t row = 0; row < plane_height; ++row) {
                // Copia bytes entre dos regiones de memoria mediante std::memcpy.
                std::memcpy(
                    // Ejecuta payload.data() + destination_offset,.
                    payload.data() + destination_offset,
                    // Aporta source + row * source_stride como elemento de la lista.
                    source + row * source_stride,
                    // Incorpora width en esta operación.
                    width
                );
                // Actualiza destination_offset con += y width.
                destination_offset += width;
            }
        }
    }
    // Ejecuta la rama alternativa si la condición anterior fue falsa.
    else {
        // Establece bytes_per_pixel a partir de 0.
        std::size_t bytes_per_pixel = 0;
        // Selecciona una rama según format.
        switch (format) {
        // Selecciona el caso kCVPixelFormatType_422YpCbCr8_yuvs.
        case kCVPixelFormatType_422YpCbCr8_yuvs:
        // Selecciona el caso kCVPixelFormatType_422YpCbCr8.
        case kCVPixelFormatType_422YpCbCr8:
            // Establece bytes_per_pixel a partir de 2.
            bytes_per_pixel = 2;
            // Termina el bucle o la rama actual.
            break;
        // Selecciona el caso kCVPixelFormatType_24RGB.
        case kCVPixelFormatType_24RGB:
            // Establece bytes_per_pixel a partir de 3.
            bytes_per_pixel = 3;
            // Termina el bucle o la rama actual.
            break;
        // Selecciona el caso kCVPixelFormatType_32BGRA.
        case kCVPixelFormatType_32BGRA:
            // Establece bytes_per_pixel a partir de 4.
            bytes_per_pixel = 4;
            // Termina el bucle o la rama actual.
            break;
        // Atiende los valores que no coinciden con los casos anteriores.
        default:
            // Termina el bucle o la rama actual.
            break;
        }

        // Establece source_stride a partir de CVPixelBufferGetBytesPerRow(pixel_buffer).
        const std::size_t source_stride =
            // Ejecuta CVPixelBufferGetBytesPerRow(pixel_buffer).
            CVPixelBufferGetBytesPerRow(pixel_buffer);
        // Establece packed_stride a partir de bytes_per_pixel == 0.
        const std::size_t packed_stride = bytes_per_pixel == 0
            // Incorpora ? source_stride en esta operación.
            ? source_stride
            // Inicializa el miembro indicado por checked_product(width, bytes_per_pixel) antes del cuerpo.
            : checked_product(width, bytes_per_pixel);
        // Evalúa la condición packed_stride > std::numeric_limits<std::uint32_t>::max().
        if (packed_stride > std::numeric_limits<std::uint32_t>::max()) {
            // Interrumpe la operación con std::overflow_error("AVFoundation frame stride overflowed.").
            throw std::overflow_error("AVFoundation frame stride overflowed.");
        }
        // Establece stride (bytes entre filas de imagen) a partir de static_cast<std::uint32_t>(packed_stride).
        stride = static_cast<std::uint32_t>(packed_stride);
        // Ajusta el tamaño del contenedor mediante payload.resize.
        payload.resize(checked_product(packed_stride, height));

        // Establece source (fuente de video) a partir de static_cast<const std::byte*>(.
        const auto* source = static_cast<const std::byte*>(
            // Ejecuta CVPixelBufferGetBaseAddress(pixel_buffer).
            CVPixelBufferGetBaseAddress(pixel_buffer)
        );
        // Evalúa la condición source == nullptr || source_stride < packed_stride.
        if (source == nullptr || source_stride < packed_stride) {
            // Interrumpe la operación con std::runtime_error(.
            throw std::runtime_error(
                // Aporta el texto "AVFoundation returned an invalid pixel buffer." a la expresión.
                "AVFoundation returned an invalid pixel buffer."
            );
        }
        // Recorre según std::size_t row = 0; row < height; ++row.
        for (std::size_t row = 0; row < height; ++row) {
            // Copia bytes entre dos regiones de memoria mediante std::memcpy.
            std::memcpy(
                // Ejecuta payload.data() + row * packed_stride,.
                payload.data() + row * packed_stride,
                // Aporta source + row * source_stride como elemento de la lista.
                source + row * source_stride,
                // Incorpora packed_stride en esta operación.
                packed_stride
            );
        }
    }

    // Devuelve {.
    return {
        // Aporta true como elemento de la lista.
        true,
        // Aporta timestamp_100ns como elemento de la lista.
        timestamp_100ns,
        // Ejecuta payload.size(),.
        payload.size(),
        // Aporta stride como elemento de la lista.
        stride,
        // Transfiere el valor sin exigir una copia mediante std::move.
        std::move(payload)
    };
}

// Incorpora } // namespace orvix::capture::detail en esta operación.
}  // namespace orvix::capture::detail

// Declara la interfaz Objective-C OrvixSampleBufferDelegate.
@interface OrvixSampleBufferDelegate
    // Inicializa el miembro indicado por NSObject <AVCaptureVideoDataOutputSampleBufferDelegate> antes del cuerpo.
    : NSObject <AVCaptureVideoDataOutputSampleBufferDelegate>

// Ejecuta @property(nonatomic, assign) void* sampleQueueContext.
@property(nonatomic, assign) void* sampleQueueContext;

// Cierra la declaración o implementación de Objective-C.
@end

// Implementa la clase Objective-C OrvixSampleBufferDelegate.
@implementation OrvixSampleBufferDelegate

// Declara o implementa el método Objective-C captureOutput.
- (void)captureOutput:(AVCaptureOutput*)output
    // Incorpora didOutputSampleBuffer:(CMSampleBufferRef)sampleBuffer en esta operación.
    didOutputSampleBuffer:(CMSampleBufferRef)sampleBuffer
    // Incorpora fromConnection:(AVCaptureConnection*)connection { en esta operación.
    fromConnection:(AVCaptureConnection*)connection {
    // Incorpora static_cast<void>(output) en esta operación.
    static_cast<void>(output);
    // Incorpora static_cast<void>(connection) en esta operación.
    static_cast<void>(connection);

    // Establece queue a partir de static_cast<orvix::capture::detail::SampleQueue*>(.
    auto* queue = static_cast<orvix::capture::detail::SampleQueue*>(
        // Incorpora self.sampleQueueContext en esta operación.
        self.sampleQueueContext
    );
    // Evalúa la condición queue == nullptr.
    if (queue == nullptr) {
        // Termina la función sin un valor de retorno.
        return;
    }

    // Ejecuta el bloque protegido contra excepciones.
    try {
        // Establece timestamp (marca de tiempo) a partir de CMSampleBufferGetPresentationTimeStamp(sampleBuffer).
        const CMTime timestamp =
            // Ejecuta CMSampleBufferGetPresentationTimeStamp(sampleBuffer).
            CMSampleBufferGetPresentationTimeStamp(sampleBuffer);
        // Establece timestamp_100ns (marca de tiempo en unidades de 100 ns) a partir de CMTimeConvertScale(.
        const CMTime timestamp_100ns = CMTimeConvertScale(
            // Aporta timestamp como elemento de la lista.
            timestamp,
            // Aporta 10'000'000 como elemento de la lista.
            10'000'000,
            // Incorpora kCMTimeRoundingMethod_Default en esta operación.
            kCMTimeRoundingMethod_Default
        );

        // Establece timestamp_value a partir de CMTIME_IS_NUMERIC(timestamp_100ns).
        const std::int64_t timestamp_value =
            // Ejecuta CMTIME_IS_NUMERIC(timestamp_100ns).
            CMTIME_IS_NUMERIC(timestamp_100ns)
                // Incorpora ? timestamp_100ns.value en esta operación.
                ? timestamp_100ns.value
                // Inicializa el miembro indicado por 0 antes del cuerpo.
                : 0;
        // Establece frame (fotograma) a partir de orvix::capture::detail::frame_from_sample(.
        auto frame = orvix::capture::detail::frame_from_sample(
            // Aporta sampleBuffer como elemento de la lista.
            sampleBuffer,
            // Incorpora timestamp_value en esta operación.
            timestamp_value
        );

        {
            // Ejecuta const std::lock_guard lock(queue->mutex).
            const std::lock_guard lock(queue->mutex);
            // Evalúa la condición queue->frames.size() >= 3.
            if (queue->frames.size() >= 3) {
                // Ejecuta queue->frames.pop_front().
                queue->frames.pop_front();
            }
            // Agrega un elemento al final del contenedor mediante queue->frames.push_back.
            queue->frames.push_back(std::move(frame));
        }

        // Despierta a un hilo en espera mediante queue->ready.notify_one.
        queue->ready.notify_one();
    }
    // Maneja la excepción declarada en catch (...) {.
    catch (...) {
        {
            // Ejecuta const std::lock_guard lock(queue->mutex).
            const std::lock_guard lock(queue->mutex);
            // Establece queue->callback_failed a partir de true.
            queue->callback_failed = true;
        }
        // Despierta a un hilo en espera mediante queue->ready.notify_one.
        queue->ready.notify_one();
    }
}

// Cierra la declaración o implementación de Objective-C.
@end

// Agrupa las declaraciones bajo orvix::capture.
namespace orvix::capture {

// Limita estas declaraciones a esta unidad de compilación.
namespace {

// Introduce en este ámbito detail::SampleQueue.
using detail::SampleQueue;

// Declara el tipo NativeFormat y las operaciones o datos de su contrato.
struct NativeFormat final {
    // Declara format (formato de video) con el tipo indicado.
    VideoFormat format;
    // Inicializa native_format con nil.
    __strong AVCaptureDeviceFormat* native_format{nil};
    // Inicializa frame_duration con kCMTimeInvalid.
    CMTime frame_duration{kCMTimeInvalid};
    // Inicializa pixel_format (formato de píxeles) con inicialización por valor.
    FourCharCode pixel_format{};
};

// Define la operación utf8_string con los parámetros declarados.
std::string utf8_string(NSString* value) {
    // Evalúa la condición value == nil.
    if (value == nil) {
        // Devuelve {}.
        return {};
    }

    // Establece utf8 a partir de value.UTF8String.
    const char* utf8 = value.UTF8String;
    // Devuelve utf8 == nullptr ? std::string{} : std::string{utf8}.
    return utf8 == nullptr ? std::string{} : std::string{utf8};
}

// Define la operación native_string con los parámetros declarados.
NSString* native_string(const std::string& value) {
    // Devuelve [NSString stringWithUTF8String:value.c_str()].
    return [NSString stringWithUTF8String:value.c_str()];
}

// Define la operación fourcc_string con los parámetros declarados.
std::string fourcc_string(const FourCharCode value) {
    // Ejecuta std::string result(4, ' ').
    std::string result(4, ' ');
    // Establece result[0] a partir de static_cast<char>((value >> 24U) & 0xffU).
    result[0] = static_cast<char>((value >> 24U) & 0xffU);
    // Establece result[1] a partir de static_cast<char>((value >> 16U) & 0xffU).
    result[1] = static_cast<char>((value >> 16U) & 0xffU);
    // Establece result[2] a partir de static_cast<char>((value >> 8U) & 0xffU).
    result[2] = static_cast<char>((value >> 8U) & 0xffU);
    // Establece result[3] a partir de static_cast<char>(value & 0xffU).
    result[3] = static_cast<char>(value & 0xffU);
    // Devuelve result.
    return result;
}

// Define la operación pixel_format_name con los parámetros declarados.
std::string pixel_format_name(const FourCharCode value) {
    // Selecciona una rama según value.
    switch (value) {
    // Selecciona el caso kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange.
    case kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange:
    // Selecciona el caso kCVPixelFormatType_420YpCbCr8BiPlanarFullRange.
    case kCVPixelFormatType_420YpCbCr8BiPlanarFullRange:
        // Devuelve "NV12".
        return "NV12";
    // Selecciona el caso kCVPixelFormatType_422YpCbCr8_yuvs.
    case kCVPixelFormatType_422YpCbCr8_yuvs:
        // Devuelve "YUY2".
        return "YUY2";
    // Selecciona el caso kCVPixelFormatType_422YpCbCr8.
    case kCVPixelFormatType_422YpCbCr8:
        // Devuelve "UYVY".
        return "UYVY";
    // Selecciona el caso kCVPixelFormatType_24RGB.
    case kCVPixelFormatType_24RGB:
        // Devuelve "RGB24".
        return "RGB24";
    // Selecciona el caso kCVPixelFormatType_32BGRA.
    case kCVPixelFormatType_32BGRA:
        // Devuelve "BGRA32".
        return "BGRA32";
    // Atiende los valores que no coinciden con los casos anteriores.
    default:
        // Devuelve fourcc_string(value).
        return fourcc_string(value);
    }
}

// Define la operación packed_stride con los parámetros declarados.
std::uint32_t packed_stride(
    // Declara el parámetro value.
    const FourCharCode value,
    // Incorpora const std::uint32_t width en esta operación.
    const std::uint32_t width
// Incorpora ) noexcept { en esta operación.
) noexcept {
    // Selecciona una rama según value.
    switch (value) {
    // Selecciona el caso kCVPixelFormatType_422YpCbCr8_yuvs.
    case kCVPixelFormatType_422YpCbCr8_yuvs:
    // Selecciona el caso kCVPixelFormatType_422YpCbCr8.
    case kCVPixelFormatType_422YpCbCr8:
        // Devuelve width * 2U.
        return width * 2U;
    // Selecciona el caso kCVPixelFormatType_24RGB.
    case kCVPixelFormatType_24RGB:
        // Devuelve width * 3U.
        return width * 3U;
    // Selecciona el caso kCVPixelFormatType_32BGRA.
    case kCVPixelFormatType_32BGRA:
        // Devuelve width * 4U.
        return width * 4U;
    // Atiende los valores que no coinciden con los casos anteriores.
    default:
        // Devuelve width.
        return width;
    }
}

// Define la operación compressed_format con los parámetros declarados.
bool compressed_format(const FourCharCode value) noexcept {
    // Establece make_fourcc a partir de [](.
    constexpr auto make_fourcc = [](
        // Declara el parámetro first.
        const char first,
        // Declara el parámetro second.
        const char second,
        // Declara el parámetro third.
        const char third,
        // Incorpora const char fourth en esta operación.
        const char fourth
    // Incorpora ) constexpr { en esta operación.
    ) constexpr {
        // Devuelve el resultado de (static_cast<FourCharCode>(first) << 24U) |.
        return
            // Inicia (static_cast<FourCharCode>(first) << 24U) | con (static_cast<FourCharCode>(second) << 16U) |.
            (static_cast<FourCharCode>(first) << 24U) |
            // Inicia (static_cast<FourCharCode>(second) << 16U) | con (static_cast<FourCharCode>(third) << 8U) |.
            (static_cast<FourCharCode>(second) << 16U) |
            // Inicia (static_cast<FourCharCode>(third) << 8U) | con static_cast<FourCharCode>(fourth).
            (static_cast<FourCharCode>(third) << 8U) |
            // Incorpora static_cast<FourCharCode>(fourth) en esta operación.
            static_cast<FourCharCode>(fourth);
    };

    // Devuelve el resultado de value == make_fourcc('j', 'p', 'e', 'g') ||.
    return
        // Combina esta condición con value == make_fourcc('j', 'p', 'e', 'g') ||.
        value == make_fourcc('j', 'p', 'e', 'g') ||
        // Combina esta condición con value == make_fourcc('m', 'j', 'p', 'a') ||.
        value == make_fourcc('m', 'j', 'p', 'a') ||
        // Combina esta condición con value == make_fourcc('m', 'j', 'p', 'b') ||.
        value == make_fourcc('m', 'j', 'p', 'b') ||
        // Combina esta condición con value == make_fourcc('a', 'v', 'c', '1') ||.
        value == make_fourcc('a', 'v', 'c', '1') ||
        // Ejecuta value == make_fourcc('h', 'v', 'c', '1').
        value == make_fourcc('h', 'v', 'c', '1');
}

// Define la operación frame_rate_ratio con los parámetros declarados.
std::pair<std::uint32_t, std::uint32_t> frame_rate_ratio(
    // Incorpora const double frame_rate en esta operación.
    const double frame_rate
) {
    // Establece scaled a partir de static_cast<std::uint32_t>(.
    const auto scaled = static_cast<std::uint32_t>(
        // Ejecuta std::llround(frame_rate * 1000.0).
        std::llround(frame_rate * 1000.0)
    );
    // Establece divisor a partir de std::gcd(scaled, 1000U).
    const std::uint32_t divisor = std::gcd(scaled, 1000U);
    // Devuelve {scaled / divisor, 1000U / divisor}.
    return {scaled / divisor, 1000U / divisor};
}

// Define la operación enumerate_formats con los parámetros declarados.
std::vector<NativeFormat> enumerate_formats(
    // Declara el parámetro device (dispositivo).
    AVCaptureDevice* device,
    // Incorpora const VideoFormatTarget& target en esta operación.
    const VideoFormatTarget& target
) {
    // Declara formats con el tipo indicado.
    std::vector<NativeFormat> formats;

    // Recorre según AVCaptureDeviceFormat* native_format in device.formats.
    for (AVCaptureDeviceFormat* native_format in device.formats) {
        // Establece description a partir de native_format.formatDescription.
        const CMFormatDescriptionRef description =
            // Incorpora native_format.formatDescription en esta operación.
            native_format.formatDescription;
        // Establece dimensions a partir de CMVideoFormatDescriptionGetDimensions(description).
        const CMVideoDimensions dimensions =
            // Ejecuta CMVideoFormatDescriptionGetDimensions(description).
            CMVideoFormatDescriptionGetDimensions(description);
        // Establece subtype a partir de CMFormatDescriptionGetMediaSubType(description).
        const FourCharCode subtype =
            // Ejecuta CMFormatDescriptionGetMediaSubType(description).
            CMFormatDescriptionGetMediaSubType(description);

        // Recorre según AVFrameRateRange* range in native_format.videoSupportedFrameRateRanges.
        for (AVFrameRateRange* range in native_format.videoSupportedFrameRateRanges) {
            // Establece selected_rate a partir de std::clamp(.
            const double selected_rate = std::clamp(
                // Aporta static_cast<double>(target.frames_per_second) como elemento de la lista.
                static_cast<double>(target.frames_per_second),
                // Aporta range.minFrameRate como elemento de la lista.
                range.minFrameRate,
                // Incorpora range.maxFrameRate en esta operación.
                range.maxFrameRate
            );

            // Evalúa la condición selected_rate <= 0.0.
            if (selected_rate <= 0.0) {
                // Pasa a la siguiente iteración.
                continue;
            }

            // Establece const auto [numerator, denominator] a partir de frame_rate_ratio(selected_rate).
            const auto [numerator, denominator] =
                // Ejecuta frame_rate_ratio(selected_rate).
                frame_rate_ratio(selected_rate);
            // Establece frame_duration a partir de CMTimeMake(.
            const CMTime frame_duration = CMTimeMake(
                // Aporta static_cast<std::int64_t>(denominator) como elemento de la lista.
                static_cast<std::int64_t>(denominator),
                // Incorpora static_cast<std::int32_t>(numerator) en esta operación.
                static_cast<std::int32_t>(numerator)
            );

            // Agrega un elemento al final del contenedor mediante formats.push_back.
            formats.push_back({
                {
                    // Ejecuta formats.size(),.
                    formats.size(),
                    // Aporta static_cast<std::uint32_t>(dimensions.width) como elemento de la lista.
                    static_cast<std::uint32_t>(dimensions.width),
                    // Aporta static_cast<std::uint32_t>(dimensions.height) como elemento de la lista.
                    static_cast<std::uint32_t>(dimensions.height),
                    // Aporta numerator como elemento de la lista.
                    numerator,
                    // Aporta denominator como elemento de la lista.
                    denominator,
                    // Ejecuta pixel_format_name(subtype),.
                    pixel_format_name(subtype),
                    // Ejecuta compressed_format(subtype).
                    compressed_format(subtype)
                },
                // Aporta native_format como elemento de la lista.
                native_format,
                // Aporta frame_duration como elemento de la lista.
                frame_duration,
                // Incorpora subtype en esta operación.
                subtype
            });
        }
    }

    // Devuelve formats.
    return formats;
}

// Define la operación require_camera_permission con los parámetros declarados.
void require_camera_permission() {
    // Establece status a partir de [AVCaptureDevice authorizationStatusForMediaType:AVMediaTypeVideo].
    AVAuthorizationStatus status =
        // Envía el mensaje de Objective-C indicado por [AVCaptureDevice authorizationStatusForMediaType:AVMediaTypeVideo].
        [AVCaptureDevice authorizationStatusForMediaType:AVMediaTypeVideo];

    // Evalúa la condición status == AVAuthorizationStatusNotDetermined.
    if (status == AVAuthorizationStatusNotDetermined) {
        // Establece permission_ready a partir de dispatch_semaphore_create(0).
        dispatch_semaphore_t permission_ready = dispatch_semaphore_create(0);
        // Establece granted a partir de NO.
        __block BOOL granted = NO;

        // Envía el mensaje de Objective-C indicado por [AVCaptureDevice requestAccessForMediaType:AVMediaTypeVideo.
        [AVCaptureDevice requestAccessForMediaType:AVMediaTypeVideo
            // Incorpora completionHandler:^(BOOL access_granted) { en esta operación.
            completionHandler:^(BOOL access_granted) {
                // Establece granted a partir de access_granted.
                granted = access_granted;
                // Ejecuta dispatch_semaphore_signal(permission_ready).
                dispatch_semaphore_signal(permission_ready);
            }];

        // Establece timeout (tiempo máximo de espera) a partir de dispatch_time(.
        const dispatch_time_t timeout = dispatch_time(
            // Aporta DISPATCH_TIME_NOW como elemento de la lista.
            DISPATCH_TIME_NOW,
            // Incorpora static_cast<std::int64_t>(30) * NSEC_PER_SEC en esta operación.
            static_cast<std::int64_t>(30) * NSEC_PER_SEC
        );

        // Evalúa la condición dispatch_semaphore_wait(permission_ready, timeout) != 0.
        if (dispatch_semaphore_wait(permission_ready, timeout) != 0) {
            // Interrumpe la operación con DiagnosticError(.
            throw DiagnosticError(
                // Aporta el texto "ORV-CAP-501" a la expresión.
                "ORV-CAP-501",
                // Aporta el texto "camera_permission_timeout" a la expresión.
                "camera_permission_timeout",
                // Aporta el texto "macOS did not return a camera permission decision." a la expresión.
                "macOS did not return a camera permission decision.",
                // Incorpora 71 en esta operación.
                71
            );
        }

        // Establece status a partir de granted.
        status = granted
            // Incorpora ? AVAuthorizationStatusAuthorized en esta operación.
            ? AVAuthorizationStatusAuthorized
            // Inicializa el miembro indicado por AVAuthorizationStatusDenied antes del cuerpo.
            : AVAuthorizationStatusDenied;
    }

    // Evalúa la condición status != AVAuthorizationStatusAuthorized.
    if (status != AVAuthorizationStatusAuthorized) {
        // Interrumpe la operación con DiagnosticError(.
        throw DiagnosticError(
            // Aporta el texto "ORV-CAP-501" a la expresión.
            "ORV-CAP-501",
            // Aporta el texto "camera_permission_denied" a la expresión.
            "camera_permission_denied",
            // Aporta el texto "Camera access is disabled in macOS Privacy & Security settings." a la expresión.
            "Camera access is disabled in macOS Privacy & Security settings.",
            // Incorpora 71 en esta operación.
            71
        );
    }
}

// Define la operación find_device con los parámetros declarados.
AVCaptureDevice* find_device(const std::string& unique_id) {
// Aplica la directiva del compilador clang diagnostic push.
#pragma clang diagnostic push
// Aplica la directiva del compilador clang diagnostic ignored "-Wdeprecated-declarations".
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    // Establece devices (dispositivos detectados) a partir de [AVCaptureDevice devicesWithMediaType:AVMediaTypeVideo].
    NSArray<AVCaptureDevice*>* devices =
        // Envía el mensaje de Objective-C indicado por [AVCaptureDevice devicesWithMediaType:AVMediaTypeVideo].
        [AVCaptureDevice devicesWithMediaType:AVMediaTypeVideo];
// Aplica la directiva del compilador clang diagnostic pop.
#pragma clang diagnostic pop

    // Establece requested_id a partir de native_string(unique_id).
    NSString* requested_id = native_string(unique_id);
    // Recorre según AVCaptureDevice* device in devices.
    for (AVCaptureDevice* device in devices) {
        // Evalúa la condición [device.uniqueID isEqualToString:requested_id].
        if ([device.uniqueID isEqualToString:requested_id]) {
            // Devuelve device.
            return device;
        }
    }

    // Devuelve nil.
    return nil;
}

// Declara el tipo AvFoundationFrameReader y las operaciones o datos de su contrato.
class AvFoundationFrameReader final : public FrameReader {
// Expone los miembros de la interfaz.
public:
    // Ejecuta AvFoundationFrameReader(.
    AvFoundationFrameReader(
        // Declara el parámetro queue.
        SampleQueue& queue,
        // Incorpora AVCaptureDevice* device en esta operación.
        AVCaptureDevice* device
    )
        // Inicializa el miembro indicado por queue_(queue), antes del cuerpo.
        : queue_(queue),
          // Ejecuta device_(device) {}.
          device_(device) {}

    // Define la operación read_next con los parámetros declarados.
    FrameReadResult read_next() override {
        // Ejecuta std::unique_lock lock(queue_.mutex).
        std::unique_lock lock(queue_.mutex);
        // Establece ready a partir de queue_.ready.wait_for(.
        const bool ready = queue_.ready.wait_for(
            // Aporta lock como elemento de la lista.
            lock,
            // Ejecuta std::chrono::milliseconds(250),.
            std::chrono::milliseconds(250),
            // Envía el mensaje de Objective-C indicado por [this] {.
            [this] {
                // Devuelve !queue_.frames.empty() || queue_.callback_failed.
                return !queue_.frames.empty() || queue_.callback_failed;
            }
        );

        // Evalúa la condición !ready.
        if (!ready) {
            // Evalúa la condición !device_.connected.
            if (!device_.connected) {
                // Interrumpe la operación con DiagnosticError(.
                throw DiagnosticError(
                    // Aporta el texto "ORV-CAP-503" a la expresión.
                    "ORV-CAP-503",
                    // Aporta el texto "camera_disconnected" a la expresión.
                    "camera_disconnected",
                    // Aporta el texto "The AVFoundation camera was disconnected during capture." a la expresión.
                    "The AVFoundation camera was disconnected during capture.",
                    // Incorpora 74 en esta operación.
                    74
                );
            }
            // Devuelve {}.
            return {};
        }

        // Evalúa la condición queue_.callback_failed.
        if (queue_.callback_failed) {
            // Interrumpe la operación con DiagnosticError(.
            throw DiagnosticError(
                // Aporta el texto "ORV-CAP-504" a la expresión.
                "ORV-CAP-504",
                // Aporta el texto "frame_read_failed" a la expresión.
                "frame_read_failed",
                // Aporta el texto "The AVFoundation sample callback failed." a la expresión.
                "The AVFoundation sample callback failed.",
                // Incorpora 75 en esta operación.
                75
            );
        }

        // Establece frame (fotograma) a partir de std::move(queue_.frames.front()).
        FrameReadResult frame = std::move(queue_.frames.front());
        // Ejecuta queue_.frames.pop_front().
        queue_.frames.pop_front();
        // Devuelve frame.
        return frame;
    }

// Restringe estos miembros a la implementación.
private:
    // Declara queue_ con el tipo indicado.
    SampleQueue& queue_;
    // Declara device_ (dispositivo) con el tipo indicado.
    __strong AVCaptureDevice* device_;
};

// Incorpora } // namespace en esta operación.
}  // namespace

// Declara el tipo AvFoundationCamera y las operaciones o datos de su contrato.
struct AvFoundationCamera::Impl final {
    // Inicializa device (dispositivo) con nil.
    __strong AVCaptureDevice* device{nil};
    // Inicializa session con nil.
    __strong AVCaptureSession* session{nil};
    // Inicializa output con nil.
    __strong AVCaptureVideoDataOutput* output{nil};
    // Inicializa delegate con nil.
    __strong OrvixSampleBufferDelegate* delegate{nil};
    // Inicializa callback_queue con nullptr.
    dispatch_queue_t callback_queue{nullptr};
    // Declara samples con el tipo indicado.
    std::unique_ptr<SampleQueue> samples;
    // Declara configured_format con el tipo indicado.
    VideoFormat configured_format;
    // Inicializa format_configured con false.
    bool format_configured{false};
};

// Ejecuta AvFoundationCamera::AvFoundationCamera().
AvFoundationCamera::AvFoundationCamera()
    // Inicializa el miembro indicado por impl_(std::make_unique<Impl>()) {} antes del cuerpo.
    : impl_(std::make_unique<Impl>()) {}

// Ejecuta AvFoundationCamera::~AvFoundationCamera() {.
AvFoundationCamera::~AvFoundationCamera() {
    // Cierra el recurso mediante close.
    close();
}

// Define la operación AvFoundationCamera::open con los parámetros declarados.
void AvFoundationCamera::open(const CameraDevice& device) {
    // Delimita el grupo de liberación automática de objetos.
    @autoreleasepool {
        // Evalúa la condición is_open().
        if (is_open()) {
            // Interrumpe la operación con std::logic_error("A camera is already open.").
            throw std::logic_error("A camera is already open.");
        }

        // Evalúa la condición device.symbolic_link.empty().
        if (device.symbolic_link.empty()) {
            // Interrumpe la operación con std::invalid_argument(.
            throw std::invalid_argument(
                // Aporta el texto "The selected camera does not provide a unique identifier." a la expresión.
                "The selected camera does not provide a unique identifier."
            );
        }

        // Ejecuta require_camera_permission().
        require_camera_permission();

        // Establece capture_device a partir de find_device(device.symbolic_link).
        AVCaptureDevice* capture_device = find_device(device.symbolic_link);
        // Evalúa la condición capture_device == nil.
        if (capture_device == nil) {
            // Interrumpe la operación con DiagnosticError(.
            throw DiagnosticError(
                // Aporta el texto "ORV-CAP-501" a la expresión.
                "ORV-CAP-501",
                // Aporta el texto "camera_open_failed" a la expresión.
                "camera_open_failed",
                // Aporta el texto "The selected AVFoundation camera is no longer available." a la expresión.
                "The selected AVFoundation camera is no longer available.",
                // Incorpora 71 en esta operación.
                71
            );
        }

        // Establece input_error a partir de nil.
        NSError* input_error = nil;
        // Establece input a partir de [AVCaptureDeviceInput deviceInputWithDevice:capture_device.
        AVCaptureDeviceInput* input =
            // Envía el mensaje de Objective-C indicado por [AVCaptureDeviceInput deviceInputWithDevice:capture_device.
            [AVCaptureDeviceInput deviceInputWithDevice:capture_device
                                                   // Incorpora error:&input_error] en esta operación.
                                                   error:&input_error];

        // Evalúa la condición input == nil.
        if (input == nil) {
            // Interrumpe la operación con std::runtime_error(.
            throw std::runtime_error(
                // Incorpora "AVCaptureDeviceInput failed: " + en esta operación.
                "AVCaptureDeviceInput failed: " +
                // Ejecuta utf8_string(input_error.localizedDescription).
                utf8_string(input_error.localizedDescription)
            );
        }

        // Establece session a partir de [[AVCaptureSession alloc] init].
        AVCaptureSession* session = [[AVCaptureSession alloc] init];
        // Evalúa la condición ![session canAddInput:input].
        if (![session canAddInput:input]) {
            // Interrumpe la operación con std::runtime_error(.
            throw std::runtime_error(
                // Aporta el texto "AVFoundation rejected the selected camera input." a la expresión.
                "AVFoundation rejected the selected camera input."
            );
        }

        // Envía el mensaje de Objective-C indicado por [session addInput:input].
        [session addInput:input];
        // Establece impl_->device a partir de capture_device.
        impl_->device = capture_device;
        // Establece impl_->session a partir de session.
        impl_->session = session;
    }
}

// Define la operación AvFoundationCamera::close con los parámetros declarados.
void AvFoundationCamera::close() noexcept {
    // Delimita el grupo de liberación automática de objetos.
    @autoreleasepool {
        // Evalúa la condición impl_ == nullptr.
        if (impl_ == nullptr) {
            // Termina la función sin un valor de retorno.
            return;
        }

        // Evalúa la condición impl_->session.running.
        if (impl_->session.running) {
            // Envía el mensaje de Objective-C indicado por [impl_->session stopRunning].
            [impl_->session stopRunning];
        }

        // Envía el mensaje de Objective-C indicado por [impl_->output setSampleBufferDelegate:nil queue:nullptr].
        [impl_->output setSampleBufferDelegate:nil queue:nullptr];
        // Establece impl_->delegate.sampleQueueContext a partir de nullptr.
        impl_->delegate.sampleQueueContext = nullptr;

        // Evalúa la condición impl_->callback_queue != nullptr.
        if (impl_->callback_queue != nullptr) {
            // Ejecuta dispatch_sync(impl_->callback_queue, ^{}).
            dispatch_sync(impl_->callback_queue, ^{});
        }
        // Establece impl_->delegate a partir de nil.
        impl_->delegate = nil;
        // Establece impl_->output a partir de nil.
        impl_->output = nil;
        // Establece impl_->session a partir de nil.
        impl_->session = nil;
        // Establece impl_->device a partir de nil.
        impl_->device = nil;
        // Establece impl_->callback_queue a partir de nullptr.
        impl_->callback_queue = nullptr;
        // Reinicia el recurso o estado mediante impl_->samples.reset.
        impl_->samples.reset();
        // Establece impl_->configured_format a partir de {}.
        impl_->configured_format = {};
        // Establece impl_->format_configured a partir de false.
        impl_->format_configured = false;
    }
}

// Define la operación AvFoundationCamera::available_formats con los parámetros declarados.
std::vector<VideoFormat> AvFoundationCamera::available_formats() const {
    // Delimita el grupo de liberación automática de objetos.
    @autoreleasepool {
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
            // Aporta impl_->device como elemento de la lista.
            impl_->device,
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
}

// Define la operación AvFoundationCamera::configure con los parámetros declarados.
VideoFormat AvFoundationCamera::configure(const VideoFormatTarget& target) {
    // Delimita el grupo de liberación automática de objetos.
    @autoreleasepool {
        // Evalúa la condición !is_open().
        if (!is_open()) {
            // Interrumpe la operación con std::logic_error(.
            throw std::logic_error(
                // Aporta el texto "A camera must be open before configuring a video format." a la expresión.
                "A camera must be open before configuring a video format."
            );
        }

        // Establece native_formats a partir de enumerate_formats(impl_->device, target).
        const auto native_formats = enumerate_formats(impl_->device, target);
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

        // Establece configuration_error a partir de nil.
        NSError* configuration_error = nil;
        // Evalúa la condición ![impl_->device lockForConfiguration:&configuration_error].
        if (![impl_->device lockForConfiguration:&configuration_error]) {
            // Interrumpe la operación con std::runtime_error(.
            throw std::runtime_error(
                // Incorpora "AVCaptureDevice::lockForConfiguration failed: " + en esta operación.
                "AVCaptureDevice::lockForConfiguration failed: " +
                // Ejecuta utf8_string(configuration_error.localizedDescription).
                utf8_string(configuration_error.localizedDescription)
            );
        }

        // Establece impl_->device.activeFormat a partir de native.native_format.
        impl_->device.activeFormat = native.native_format;
        // Establece impl_->device.activeVideoMinFrameDuration a partir de native.frame_duration.
        impl_->device.activeVideoMinFrameDuration = native.frame_duration;
        // Establece impl_->device.activeVideoMaxFrameDuration a partir de native.frame_duration.
        impl_->device.activeVideoMaxFrameDuration = native.frame_duration;
        // Envía el mensaje de Objective-C indicado por [impl_->device unlockForConfiguration].
        [impl_->device unlockForConfiguration];

        // Establece samples a partir de std::make_unique<SampleQueue>().
        auto samples = std::make_unique<SampleQueue>();
        // Establece delegate a partir de [[OrvixSampleBufferDelegate alloc] init].
        OrvixSampleBufferDelegate* delegate =
            // Declara el atributo de compilación [[OrvixSampleBufferDelegate alloc] init].
            [[OrvixSampleBufferDelegate alloc] init];
        // Establece delegate.sampleQueueContext a partir de samples.get().
        delegate.sampleQueueContext = samples.get();

        // Establece output a partir de [[AVCaptureVideoDataOutput alloc] init].
        AVCaptureVideoDataOutput* output =
            // Declara el atributo de compilación [[AVCaptureVideoDataOutput alloc] init].
            [[AVCaptureVideoDataOutput alloc] init];
        // Establece output.alwaysDiscardsLateVideoFrames a partir de YES.
        output.alwaysDiscardsLateVideoFrames = YES;

        // Establece preferred_pixel_formats a partir de @[.
        NSArray<NSNumber*>* preferred_pixel_formats = @[
            // Aporta @(kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange) como elemento de la lista.
            @(kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange),
            // Aporta @(kCVPixelFormatType_420YpCbCr8BiPlanarFullRange) como elemento de la lista.
            @(kCVPixelFormatType_420YpCbCr8BiPlanarFullRange),
            // Aporta @(kCVPixelFormatType_32BGRA) como elemento de la lista.
            @(kCVPixelFormatType_32BGRA),
            // Aporta @(kCVPixelFormatType_422YpCbCr8_yuvs) como elemento de la lista.
            @(kCVPixelFormatType_422YpCbCr8_yuvs),
            // Aporta @(kCVPixelFormatType_422YpCbCr8) como elemento de la lista.
            @(kCVPixelFormatType_422YpCbCr8),
            // Incorpora @(kCVPixelFormatType_24RGB) en esta operación.
            @(kCVPixelFormatType_24RGB)
        ];
        // Establece requested_pixel_format a partir de nil.
        NSNumber* requested_pixel_format = nil;
        // Recorre según NSNumber* candidate in preferred_pixel_formats.
        for (NSNumber* candidate in preferred_pixel_formats) {
            // Evalúa la condición [.
            if ([
                // Incorpora output.availableVideoCVPixelFormatTypes en esta operación.
                output.availableVideoCVPixelFormatTypes
                // Incorpora containsObject:candidate en esta operación.
                containsObject:candidate
            ]) {
                // Establece requested_pixel_format a partir de candidate.
                requested_pixel_format = candidate;
                // Termina el bucle o la rama actual.
                break;
            }
        }

        // Evalúa la condición requested_pixel_format == nil.
        if (requested_pixel_format == nil) {
            // Establece delegate.sampleQueueContext a partir de nullptr.
            delegate.sampleQueueContext = nullptr;
            // Interrumpe la operación con std::runtime_error(.
            throw std::runtime_error(
                // Aporta el texto "AVFoundation did not expose an ORVIX display format." a la expresión.
                "AVFoundation did not expose an ORVIX display format."
            );
        }
        // Establece output_pixel_format a partir de requested_pixel_format.unsignedIntValue.
        const FourCharCode output_pixel_format =
            // Incorpora requested_pixel_format.unsignedIntValue en esta operación.
            requested_pixel_format.unsignedIntValue;

        // Establece output.videoSettings a partir de @{.
        output.videoSettings = @{
            // Inicia (NSString*)kCVPixelBufferPixelFormatTypeKey: con requested_pixel_format.
            (NSString*)kCVPixelBufferPixelFormatTypeKey:
                // Incorpora requested_pixel_format en esta operación.
                requested_pixel_format
        };

        // Establece callback_queue a partir de dispatch_queue_create(.
        dispatch_queue_t callback_queue = dispatch_queue_create(
            // Aporta el texto "com.orvix.capture.frames" a la expresión.
            "com.orvix.capture.frames",
            // Incorpora DISPATCH_QUEUE_SERIAL en esta operación.
            DISPATCH_QUEUE_SERIAL
        );
        // Envía el mensaje de Objective-C indicado por [output setSampleBufferDelegate:delegate queue:callback_queue].
        [output setSampleBufferDelegate:delegate queue:callback_queue];

        // Evalúa la condición ![impl_->session canAddOutput:output].
        if (![impl_->session canAddOutput:output]) {
            // Envía el mensaje de Objective-C indicado por [output setSampleBufferDelegate:nil queue:nullptr].
            [output setSampleBufferDelegate:nil queue:nullptr];
            // Establece delegate.sampleQueueContext a partir de nullptr.
            delegate.sampleQueueContext = nullptr;
            // Interrumpe la operación con std::runtime_error(.
            throw std::runtime_error(
                // Aporta el texto "AVFoundation rejected the video data output." a la expresión.
                "AVFoundation rejected the video data output."
            );
        }

        // Envía el mensaje de Objective-C indicado por [impl_->session addOutput:output].
        [impl_->session addOutput:output];
        // Establece impl_->samples a partir de std::move(samples).
        impl_->samples = std::move(samples);
        // Establece impl_->delegate a partir de delegate.
        impl_->delegate = delegate;
        // Establece impl_->output a partir de output.
        impl_->output = output;
        // Establece impl_->callback_queue a partir de callback_queue.
        impl_->callback_queue = callback_queue;
        // Establece configured a partir de selected.
        VideoFormat configured = selected;
        // Establece configured.pixel_format (formato de píxeles) a partir de pixel_format_name(output_pixel_format).
        configured.pixel_format = pixel_format_name(output_pixel_format);
        // Establece configured.compressed a partir de false.
        configured.compressed = false;
        // Establece configured.stride (bytes entre filas de imagen) a partir de packed_stride(.
        configured.stride = packed_stride(
            // Aporta output_pixel_format como elemento de la lista.
            output_pixel_format,
            // Incorpora configured.width en esta operación.
            configured.width
        );
        // Establece impl_->configured_format a partir de configured.
        impl_->configured_format = configured;
        // Establece impl_->format_configured a partir de true.
        impl_->format_configured = true;
        // Devuelve configured.
        return configured;
    }
}

// Define la operación AvFoundationCamera::capture_frames con los parámetros declarados.
CaptureSummary AvFoundationCamera::capture_frames(
    // Declara el parámetro requested_frames.
    const std::size_t requested_frames,
    // Incorpora FrameSink* const sink en esta operación.
    FrameSink* const sink
) {
    // Delimita el grupo de liberación automática de objetos.
    @autoreleasepool {
        // Evalúa la condición !is_open().
        if (!is_open()) {
            // Interrumpe la operación con std::logic_error(.
            throw std::logic_error(
                // Aporta el texto "A camera must be open before capturing frames." a la expresión.
                "A camera must be open before capturing frames."
            );
        }

        // Evalúa la condición !impl_->format_configured || impl_->samples == nullptr.
        if (!impl_->format_configured || impl_->samples == nullptr) {
            // Interrumpe la operación con std::logic_error(.
            throw std::logic_error(
                // Aporta el texto "A video format must be configured before capturing frames." a la expresión.
                "A video format must be configured before capturing frames."
            );
        }

        {
            // Ejecuta const std::lock_guard lock(impl_->samples->mutex).
            const std::lock_guard lock(impl_->samples->mutex);
            // Vacía el contenedor mediante impl_->samples->frames.clear.
            impl_->samples->frames.clear();
            // Establece impl_->samples->callback_failed a partir de false.
            impl_->samples->callback_failed = false;
        }

        // Envía el mensaje de Objective-C indicado por [impl_->session startRunning].
        [impl_->session startRunning];

        // Ejecuta el bloque protegido contra excepciones.
        try {
            // Define la operación reader con los parámetros declarados.
            AvFoundationFrameReader reader(
                // Aporta *impl_->samples como elemento de la lista.
                *impl_->samples,
                // Incorpora impl_->device en esta operación.
                impl_->device
            );
            // Establece summary (resumen de la sesión) a partir de ContinuousFrameCapture::run(.
            CaptureSummary summary = ContinuousFrameCapture::run(
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
            // Envía el mensaje de Objective-C indicado por [impl_->session stopRunning].
            [impl_->session stopRunning];
            // Devuelve summary.
            return summary;
        }
        // Maneja la excepción declarada en catch (...) {.
        catch (...) {
            // Envía el mensaje de Objective-C indicado por [impl_->session stopRunning].
            [impl_->session stopRunning];
            // Vuelve a propagar la excepción.
            throw;
        }
    }
}

// Define la operación AvFoundationCamera::is_open con los parámetros declarados.
bool AvFoundationCamera::is_open() const noexcept {
    // Devuelve el resultado de impl_ != nullptr &&.
    return
        // Combina esta condición con impl_ != nullptr &&.
        impl_ != nullptr &&
        // Combina esta condición con impl_->device != nil &&.
        impl_->device != nil &&
        // Evalúa impl_->session != nil como parte de la condición.
        impl_->session != nil;
}

// Incorpora } // namespace orvix::capture en esta operación.
}  // namespace orvix::capture
