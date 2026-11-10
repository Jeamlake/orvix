// Incluye las declaraciones de media foundation camera de ORVIX.
#include "orvix/capture/media_foundation_camera.hpp"

// Incluye las declaraciones de continuous frame capture de ORVIX.
#include "orvix/capture/continuous_frame_capture.hpp"
// Incluye las declaraciones de diagnostic error de ORVIX.
#include "orvix/capture/diagnostic_error.hpp"
// Incluye las declaraciones de hresult error de ORVIX.
#include "orvix/capture/hresult_error.hpp"
// Incluye las declaraciones de media foundation runtime de ORVIX.
#include "orvix/capture/media_foundation_runtime.hpp"
// Incluye las declaraciones de video format selector de ORVIX.
#include "orvix/capture/video_format_selector.hpp"

// Importa las declaraciones de Windows.h para esta unidad.
#include <Windows.h>
// Importa las declaraciones de mfapi.h para esta unidad.
#include <mfapi.h>
// Importa las declaraciones de mferror.h para esta unidad.
#include <mferror.h>
// Importa las declaraciones de mfidl.h para esta unidad.
#include <mfidl.h>
// Importa las declaraciones de mfreadwrite.h para esta unidad.
#include <mfreadwrite.h>
// Importa las declaraciones de wrl/client.h para esta unidad.
#include <wrl/client.h>

// Incluye cstddef para usar tipos de tamaño y bytes.
#include <cstddef>
// Incluye cstdint para usar enteros de tamaño fijo.
#include <cstdint>
// Incluye cstring para usar operaciones de memoria y cadenas de C.
#include <cstring>
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

// Introduce en este ámbito Microsoft::WRL::ComPtr.
using Microsoft::WRL::ComPtr;

// Establece kAllStreams a partir de static_cast<DWORD>(MF_SOURCE_READER_ALL_STREAMS).
constexpr DWORD kAllStreams =
    // Incorpora static_cast<DWORD>(MF_SOURCE_READER_ALL_STREAMS) en esta operación.
    static_cast<DWORD>(MF_SOURCE_READER_ALL_STREAMS);
// Establece kFirstVideoStream a partir de static_cast<DWORD>(MF_SOURCE_READER_FIRST_VIDEO_STREAM).
constexpr DWORD kFirstVideoStream =
    // Incorpora static_cast<DWORD>(MF_SOURCE_READER_FIRST_VIDEO_STREAM) en esta operación.
    static_cast<DWORD>(MF_SOURCE_READER_FIRST_VIDEO_STREAM);

// Define la operación utf8_to_wide con los parámetros declarados.
std::wstring utf8_to_wide(const std::string& value) {
    // Evalúa la condición value.empty().
    if (value.empty()) {
        // Devuelve {}.
        return {};
    }

    // Establece required a partir de MultiByteToWideChar(.
    const int required = MultiByteToWideChar(
        // Aporta CP_UTF8 como elemento de la lista.
        CP_UTF8,
        // Aporta MB_ERR_INVALID_CHARS como elemento de la lista.
        MB_ERR_INVALID_CHARS,
        // Ejecuta value.c_str(),.
        value.c_str(),
        // Aporta -1 como elemento de la lista.
        -1,
        // Aporta nullptr como elemento de la lista.
        nullptr,
        // Incorpora 0 en esta operación.
        0
    );

    // Evalúa la condición required <= 0.
    if (required <= 0) {
        // Interrumpe la operación con HResultError(.
        throw HResultError(
            // Ejecuta HRESULT_FROM_WIN32(GetLastError()),.
            HRESULT_FROM_WIN32(GetLastError()),
            // Aporta el texto "MultiByteToWideChar(size)" a la expresión.
            "MultiByteToWideChar(size)"
        );
    }

    // Ejecuta std::wstring result(static_cast<std::size_t>(required), L'\0').
    std::wstring result(static_cast<std::size_t>(required), L'\0');

    // Establece written a partir de MultiByteToWideChar(.
    const int written = MultiByteToWideChar(
        // Aporta CP_UTF8 como elemento de la lista.
        CP_UTF8,
        // Aporta MB_ERR_INVALID_CHARS como elemento de la lista.
        MB_ERR_INVALID_CHARS,
        // Ejecuta value.c_str(),.
        value.c_str(),
        // Aporta -1 como elemento de la lista.
        -1,
        // Ejecuta result.data(),.
        result.data(),
        // Incorpora required en esta operación.
        required
    );

    // Evalúa la condición written <= 0.
    if (written <= 0) {
        // Interrumpe la operación con HResultError(.
        throw HResultError(
            // Ejecuta HRESULT_FROM_WIN32(GetLastError()),.
            HRESULT_FROM_WIN32(GetLastError()),
            // Aporta el texto "MultiByteToWideChar(convert)" a la expresión.
            "MultiByteToWideChar(convert)"
        );
    }

    // Evalúa la condición !result.empty() && result.back() == L'\0'.
    if (!result.empty() && result.back() == L'\0') {
        // Ejecuta result.pop_back().
        result.pop_back();
    }

    // Devuelve result.
    return result;
}

// Define la operación guid_equals con los parámetros declarados.
bool guid_equals(const GUID& left, const GUID& right) noexcept {
    // Devuelve IsEqualGUID(left, right) != FALSE.
    return IsEqualGUID(left, right) != FALSE;
}

// Define la operación pixel_format_name con los parámetros declarados.
std::string pixel_format_name(const GUID& subtype) {
    // Evalúa la condición guid_equals(subtype, MFVideoFormat_NV12).
    if (guid_equals(subtype, MFVideoFormat_NV12)) {
        // Devuelve "NV12".
        return "NV12";
    }

    // Evalúa la condición guid_equals(subtype, MFVideoFormat_YUY2).
    if (guid_equals(subtype, MFVideoFormat_YUY2)) {
        // Devuelve "YUY2".
        return "YUY2";
    }

    // Evalúa la condición guid_equals(subtype, MFVideoFormat_UYVY).
    if (guid_equals(subtype, MFVideoFormat_UYVY)) {
        // Devuelve "UYVY".
        return "UYVY";
    }

    // Evalúa la condición guid_equals(subtype, MFVideoFormat_I420).
    if (guid_equals(subtype, MFVideoFormat_I420)) {
        // Devuelve "I420".
        return "I420";
    }

    // Evalúa la condición guid_equals(subtype, MFVideoFormat_IYUV).
    if (guid_equals(subtype, MFVideoFormat_IYUV)) {
        // Devuelve "IYUV".
        return "IYUV";
    }

    // Evalúa la condición guid_equals(subtype, MFVideoFormat_YV12).
    if (guid_equals(subtype, MFVideoFormat_YV12)) {
        // Devuelve "YV12".
        return "YV12";
    }

    // Evalúa la condición guid_equals(subtype, MFVideoFormat_RGB24).
    if (guid_equals(subtype, MFVideoFormat_RGB24)) {
        // Devuelve "RGB24".
        return "RGB24";
    }

    // Evalúa la condición guid_equals(subtype, MFVideoFormat_RGB32).
    if (guid_equals(subtype, MFVideoFormat_RGB32)) {
        // Devuelve "RGB32".
        return "RGB32";
    }

    // Evalúa la condición guid_equals(subtype, MFVideoFormat_ARGB32).
    if (guid_equals(subtype, MFVideoFormat_ARGB32)) {
        // Devuelve "ARGB32".
        return "ARGB32";
    }

    // Evalúa la condición guid_equals(subtype, MFVideoFormat_MJPG).
    if (guid_equals(subtype, MFVideoFormat_MJPG)) {
        // Devuelve "MJPG".
        return "MJPG";
    }

    // Evalúa la condición guid_equals(subtype, MFVideoFormat_H264).
    if (guid_equals(subtype, MFVideoFormat_H264)) {
        // Devuelve "H264".
        return "H264";
    }

    // Devuelve "UNKNOWN".
    return "UNKNOWN";
}

// Define la operación is_compressed_format con los parámetros declarados.
bool is_compressed_format(const GUID& subtype) noexcept {
    // Devuelve el resultado de guid_equals(subtype, MFVideoFormat_MJPG) ||.
    return
        // Combina esta condición con guid_equals(subtype, MFVideoFormat_MJPG) ||.
        guid_equals(subtype, MFVideoFormat_MJPG) ||
        // Ejecuta guid_equals(subtype, MFVideoFormat_H264).
        guid_equals(subtype, MFVideoFormat_H264);
}

// Define la operación packed_stride con los parámetros declarados.
std::uint32_t packed_stride(
    // Declara el parámetro pixel_format (formato de píxeles).
    const std::string& pixel_format,
    // Incorpora const std::uint32_t width en esta operación.
    const std::uint32_t width
// Incorpora ) noexcept { en esta operación.
) noexcept {
    // Evalúa la condición pixel_format == "YUY2" ||.
    if (
        // Combina esta condición con pixel_format == "YUY2" ||.
        pixel_format == "YUY2" ||
        // Evalúa pixel_format == "UYVY" como parte de la condición.
        pixel_format == "UYVY"
    ) {
        // Devuelve width * 2U.
        return width * 2U;
    }
    // Evalúa la condición pixel_format == "RGB24".
    if (pixel_format == "RGB24") {
        // Devuelve width * 3U.
        return width * 3U;
    }
    // Evalúa la condición pixel_format == "RGB32" ||.
    if (
        // Combina esta condición con pixel_format == "RGB32" ||.
        pixel_format == "RGB32" ||
        // Combina esta condición con pixel_format == "ARGB32" ||.
        pixel_format == "ARGB32" ||
        // Evalúa pixel_format == "BGRA32" como parte de la condición.
        pixel_format == "BGRA32"
    ) {
        // Devuelve width * 4U.
        return width * 4U;
    }
    // Devuelve width.
    return width;
}

// Define la operación try_read_video_format con los parámetros declarados.
bool try_read_video_format(
    // Declara el parámetro media_type.
    IMFMediaType* media_type,
    // Declara el parámetro native_type_index.
    const std::size_t native_type_index,
    // Incorpora VideoFormat& format en esta operación.
    VideoFormat& format
) {
    // Inicializa major_type con inicialización por valor.
    GUID major_type{};
    // Evalúa la condición FAILED(media_type->GetGUID(MF_MT_MAJOR_TYPE, &major_type)) ||.
    if (
        // Combina esta condición con FAILED(media_type->GetGUID(MF_MT_MAJOR_TYPE, &major_type)) ||.
        FAILED(media_type->GetGUID(MF_MT_MAJOR_TYPE, &major_type)) ||
        // Ejecuta !guid_equals(major_type, MFMediaType_Video).
        !guid_equals(major_type, MFMediaType_Video)
    ) {
        // Devuelve false.
        return false;
    }

    // Establece width (ancho en píxeles) a partir de 0.
    UINT32 width = 0;
    // Establece height (alto en píxeles) a partir de 0.
    UINT32 height = 0;
    // Evalúa la condición FAILED(MFGetAttributeSize(.
    if (FAILED(MFGetAttributeSize(
        // Aporta media_type como elemento de la lista.
        media_type,
        // Aporta MF_MT_FRAME_SIZE como elemento de la lista.
        MF_MT_FRAME_SIZE,
        // Aporta &width como elemento de la lista.
        &width,
        // Incorpora &height en esta operación.
        &height
    ))) {
        // Devuelve false.
        return false;
    }

    // Establece frame_rate_numerator a partir de 0.
    UINT32 frame_rate_numerator = 0;
    // Establece frame_rate_denominator a partir de 0.
    UINT32 frame_rate_denominator = 0;
    // Evalúa la condición FAILED(MFGetAttributeRatio(.
    if (
        // Ejecuta FAILED(MFGetAttributeRatio(.
        FAILED(MFGetAttributeRatio(
            // Aporta media_type como elemento de la lista.
            media_type,
            // Aporta MF_MT_FRAME_RATE como elemento de la lista.
            MF_MT_FRAME_RATE,
            // Aporta &frame_rate_numerator como elemento de la lista.
            &frame_rate_numerator,
            // Incorpora &frame_rate_denominator en esta operación.
            &frame_rate_denominator
        // Combina esta condición con )) ||.
        )) ||
        // Evalúa frame_rate_denominator == 0 como parte de la condición.
        frame_rate_denominator == 0
    ) {
        // Devuelve false.
        return false;
    }

    // Inicializa subtype con inicialización por valor.
    GUID subtype{};
    // Evalúa la condición FAILED(media_type->GetGUID(MF_MT_SUBTYPE, &subtype)).
    if (FAILED(media_type->GetGUID(MF_MT_SUBTYPE, &subtype))) {
        // Devuelve false.
        return false;
    }

    // Establece format.native_type_index a partir de native_type_index.
    format.native_type_index = native_type_index;
    // Establece format.width (ancho en píxeles) a partir de width.
    format.width = width;
    // Establece format.height (alto en píxeles) a partir de height.
    format.height = height;
    // Establece format.frame_rate_numerator a partir de frame_rate_numerator.
    format.frame_rate_numerator = frame_rate_numerator;
    // Establece format.frame_rate_denominator a partir de frame_rate_denominator.
    format.frame_rate_denominator = frame_rate_denominator;
    // Establece format.pixel_format (formato de píxeles) a partir de pixel_format_name(subtype).
    format.pixel_format = pixel_format_name(subtype);
    // Establece format.compressed a partir de is_compressed_format(subtype).
    format.compressed = is_compressed_format(subtype);
    // Establece format.stride (bytes entre filas de imagen) a partir de packed_stride(format.pixel_format, width).
    format.stride = packed_stride(format.pixel_format, width);

    // Devuelve true.
    return true;
}

// Declara el tipo MediaFoundationFrameReader y las operaciones o datos de su contrato.
class MediaFoundationFrameReader final : public FrameReader {
// Expone los miembros de la interfaz.
public:
    // Ejecuta MediaFoundationFrameReader(.
    MediaFoundationFrameReader(
        // Declara el parámetro source_reader.
        IMFSourceReader* source_reader,
        // Incorpora const std::uint32_t stride en esta operación.
        const std::uint32_t stride
    )
        // Inicializa el miembro indicado por source_reader_(source_reader), antes del cuerpo.
        : source_reader_(source_reader),
          // Ejecuta stride_(stride) {}.
          stride_(stride) {}

    // Define la operación read_next con los parámetros declarados.
    FrameReadResult read_next() override {
        // Establece stream_flags a partir de 0.
        DWORD stream_flags = 0;
        // Establece timestamp (marca de tiempo) a partir de 0.
        LONGLONG timestamp = 0;
        // Declara sample con el tipo indicado.
        ComPtr<IMFSample> sample;

        // Establece result a partir de source_reader_->ReadSample(.
        const HRESULT result = source_reader_->ReadSample(
            // Aporta kFirstVideoStream como elemento de la lista.
            kFirstVideoStream,
            // Aporta 0 como elemento de la lista.
            0,
            // Aporta nullptr como elemento de la lista.
            nullptr,
            // Aporta &stream_flags como elemento de la lista.
            &stream_flags,
            // Aporta &timestamp como elemento de la lista.
            &timestamp,
            // Incorpora &sample en esta operación.
            &sample
        );

        // Evalúa la condición result == MF_E_VIDEO_RECORDING_DEVICE_INVALIDATED ||.
        if (
            // Combina esta condición con result == MF_E_VIDEO_RECORDING_DEVICE_INVALIDATED ||.
            result == MF_E_VIDEO_RECORDING_DEVICE_INVALIDATED ||
            // Ejecuta result == HRESULT_FROM_WIN32(ERROR_DEVICE_NOT_CONNECTED).
            result == HRESULT_FROM_WIN32(ERROR_DEVICE_NOT_CONNECTED)
        ) {
            // Interrumpe la operación con DiagnosticError(.
            throw DiagnosticError(
                // Aporta el texto "ORV-CAP-503" a la expresión.
                "ORV-CAP-503",
                // Aporta el texto "camera_disconnected" a la expresión.
                "camera_disconnected",
                // Aporta el texto "The camera was disconnected or invalidated during capture." a la expresión.
                "The camera was disconnected or invalidated during capture.",
                // Incorpora 74 en esta operación.
                74
            );
        }

        // Evalúa la condición FAILED(result).
        if (FAILED(result)) {
            // Interrumpe la operación con HResultError(result, "IMFSourceReader::ReadSample").
            throw HResultError(result, "IMFSourceReader::ReadSample");
        }

        // Evalúa la condición (stream_flags & MF_SOURCE_READERF_ERROR) != 0.
        if ((stream_flags & MF_SOURCE_READERF_ERROR) != 0) {
            // Interrumpe la operación con DiagnosticError(.
            throw DiagnosticError(
                // Aporta el texto "ORV-CAP-503" a la expresión.
                "ORV-CAP-503",
                // Aporta el texto "camera_stream_error" a la expresión.
                "camera_stream_error",
                // Aporta el texto "The camera stream reported a device error." a la expresión.
                "The camera stream reported a device error.",
                // Incorpora 74 en esta operación.
                74
            );
        }

        // Evalúa la condición (stream_flags & MF_SOURCE_READERF_ENDOFSTREAM) != 0.
        if ((stream_flags & MF_SOURCE_READERF_ENDOFSTREAM) != 0) {
            // Interrumpe la operación con DiagnosticError(.
            throw DiagnosticError(
                // Aporta el texto "ORV-CAP-503" a la expresión.
                "ORV-CAP-503",
                // Aporta el texto "camera_stream_ended" a la expresión.
                "camera_stream_ended",
                // Aporta el texto "The camera stream ended before capture completed." a la expresión.
                "The camera stream ended before capture completed.",
                // Incorpora 74 en esta operación.
                74
            );
        }

        // Evalúa la condición (stream_flags & MF_SOURCE_READERF_NATIVEMEDIATYPECHANGED) != 0 ||.
        if (
            // Combina esta condición con (stream_flags & MF_SOURCE_READERF_NATIVEMEDIATYPECHANGED) != 0 ||.
            (stream_flags & MF_SOURCE_READERF_NATIVEMEDIATYPECHANGED) != 0 ||
            // Evalúa (stream_flags & MF_SOURCE_READERF_CURRENTMEDIATYPECHANGED) != 0 como parte de la condición.
            (stream_flags & MF_SOURCE_READERF_CURRENTMEDIATYPECHANGED) != 0
        ) {
            // Interrumpe la operación con DiagnosticError(.
            throw DiagnosticError(
                // Aporta el texto "ORV-CAP-505" a la expresión.
                "ORV-CAP-505",
                // Aporta el texto "camera_format_changed" a la expresión.
                "camera_format_changed",
                // Aporta el texto "The camera format changed during capture." a la expresión.
                "The camera format changed during capture.",
                // Incorpora 75 en esta operación.
                75
            );
        }

        // Evalúa la condición sample == nullptr.
        if (sample == nullptr) {
            // Devuelve {false, timestamp, 0}.
            return {false, timestamp, 0};
        }

        // Establece byte_count a partir de 0.
        DWORD byte_count = 0;
        // Establece length_result a partir de sample->GetTotalLength(&byte_count).
        const HRESULT length_result = sample->GetTotalLength(&byte_count);

        // Evalúa la condición FAILED(length_result).
        if (FAILED(length_result)) {
            // Interrumpe la operación con HResultError(length_result, "IMFSample::GetTotalLength").
            throw HResultError(length_result, "IMFSample::GetTotalLength");
        }

        // Declara contiguous_buffer con el tipo indicado.
        ComPtr<IMFMediaBuffer> contiguous_buffer;
        // Establece buffer_result a partir de sample->ConvertToContiguousBuffer(.
        const HRESULT buffer_result = sample->ConvertToContiguousBuffer(
            // Incorpora &contiguous_buffer en esta operación.
            &contiguous_buffer
        );
        // Evalúa la condición FAILED(buffer_result).
        if (FAILED(buffer_result)) {
            // Interrumpe la operación con HResultError(.
            throw HResultError(
                // Aporta buffer_result como elemento de la lista.
                buffer_result,
                // Aporta el texto "IMFSample::ConvertToContiguousBuffer" a la expresión.
                "IMFSample::ConvertToContiguousBuffer"
            );
        }

        // Define la operación payload con los parámetros declarados.
        std::vector<std::byte> payload(
            // Incorpora static_cast<std::size_t>(byte_count) en esta operación.
            static_cast<std::size_t>(byte_count)
        );
        // Establece data a partir de nullptr.
        BYTE* data = nullptr;
        // Establece maximum_length a partir de 0.
        DWORD maximum_length = 0;
        // Establece current_length a partir de 0.
        DWORD current_length = 0;
        // Establece lock_result a partir de contiguous_buffer->Lock(.
        const HRESULT lock_result = contiguous_buffer->Lock(
            // Aporta &data como elemento de la lista.
            &data,
            // Aporta &maximum_length como elemento de la lista.
            &maximum_length,
            // Incorpora &current_length en esta operación.
            &current_length
        );
        // Evalúa la condición FAILED(lock_result).
        if (FAILED(lock_result)) {
            // Interrumpe la operación con HResultError(lock_result, "IMFMediaBuffer::Lock").
            throw HResultError(lock_result, "IMFMediaBuffer::Lock");
        }

        // Evalúa la condición current_length != byte_count || current_length > maximum_length.
        if (current_length != byte_count || current_length > maximum_length) {
            // Ejecuta static_cast<void>(contiguous_buffer->Unlock()).
            static_cast<void>(contiguous_buffer->Unlock());
            // Interrumpe la operación con std::runtime_error(.
            throw std::runtime_error(
                // Aporta el texto "Media Foundation returned an inconsistent frame length." a la expresión.
                "Media Foundation returned an inconsistent frame length."
            );
        }

        // Evalúa la condición byte_count != 0.
        if (byte_count != 0) {
            // Copia bytes entre dos regiones de memoria mediante std::memcpy.
            std::memcpy(payload.data(), data, byte_count);
        }

        // Establece unlock_result a partir de contiguous_buffer->Unlock().
        const HRESULT unlock_result = contiguous_buffer->Unlock();
        // Evalúa la condición FAILED(unlock_result).
        if (FAILED(unlock_result)) {
            // Interrumpe la operación con HResultError(unlock_result, "IMFMediaBuffer::Unlock").
            throw HResultError(unlock_result, "IMFMediaBuffer::Unlock");
        }

        // Devuelve {.
        return {
            // Aporta true como elemento de la lista.
            true,
            // Aporta static_cast<std::int64_t>(timestamp) como elemento de la lista.
            static_cast<std::int64_t>(timestamp),
            // Aporta static_cast<std::size_t>(byte_count) como elemento de la lista.
            static_cast<std::size_t>(byte_count),
            // Aporta stride_ como elemento de la lista.
            stride_,
            // Transfiere el valor sin exigir una copia mediante std::move.
            std::move(payload)
        };
    }

// Restringe estos miembros a la implementación.
private:
    // Declara source_reader_ con el tipo indicado.
    IMFSourceReader* source_reader_;
    // Inicializa stride_ (bytes entre filas de imagen) con inicialización por valor.
    std::uint32_t stride_{};
};

// Incorpora } // namespace en esta operación.
}  // namespace

// Declara el tipo MediaFoundationCamera y las operaciones o datos de su contrato.
struct MediaFoundationCamera::Impl final {
    // Declara com_runtime con el tipo indicado.
    ComRuntime com_runtime;
    // Declara media_foundation_runtime con el tipo indicado.
    MediaFoundationRuntime media_foundation_runtime;
    // Declara media_source con el tipo indicado.
    ComPtr<IMFMediaSource> media_source;
    // Declara source_reader con el tipo indicado.
    ComPtr<IMFSourceReader> source_reader;
    // Declara configured_format con el tipo indicado.
    VideoFormat configured_format;
    // Inicializa format_configured con false.
    bool format_configured{false};
};

// Ejecuta MediaFoundationCamera::MediaFoundationCamera().
MediaFoundationCamera::MediaFoundationCamera()
    // Inicializa el miembro indicado por impl_(std::make_unique<Impl>()) {} antes del cuerpo.
    : impl_(std::make_unique<Impl>()) {}

// Ejecuta MediaFoundationCamera::~MediaFoundationCamera() {.
MediaFoundationCamera::~MediaFoundationCamera() {
    // Cierra el recurso mediante close.
    close();
}

// Define la operación MediaFoundationCamera::open con los parámetros declarados.
void MediaFoundationCamera::open(const CameraDevice& device) {
    // Evalúa la condición is_open().
    if (is_open()) {
        // Interrumpe la operación con std::logic_error("A camera is already open.").
        throw std::logic_error("A camera is already open.");
    }

    // Evalúa la condición device.symbolic_link.empty().
    if (device.symbolic_link.empty()) {
        // Interrumpe la operación con std::invalid_argument(.
        throw std::invalid_argument(
            // Aporta el texto "The selected camera does not provide a symbolic link." a la expresión.
            "The selected camera does not provide a symbolic link."
        );
    }

    // Declara attributes con el tipo indicado.
    ComPtr<IMFAttributes> attributes;
    // Establece result a partir de MFCreateAttributes(&attributes, 2).
    HRESULT result = MFCreateAttributes(&attributes, 2);

    // Evalúa la condición FAILED(result).
    if (FAILED(result)) {
        // Interrumpe la operación con HResultError(result, "MFCreateAttributes(camera source)").
        throw HResultError(result, "MFCreateAttributes(camera source)");
    }

    // Establece result a partir de attributes->SetGUID(.
    result = attributes->SetGUID(
        // Aporta MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE como elemento de la lista.
        MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE,
        // Incorpora MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID en esta operación.
        MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID
    );

    // Evalúa la condición FAILED(result).
    if (FAILED(result)) {
        // Interrumpe la operación con HResultError(.
        throw HResultError(
            // Aporta result como elemento de la lista.
            result,
            // Aporta el texto "IMFAttributes::SetGUID(video capture)" a la expresión.
            "IMFAttributes::SetGUID(video capture)"
        );
    }

    // Establece symbolic_link a partir de utf8_to_wide(device.symbolic_link).
    const std::wstring symbolic_link = utf8_to_wide(device.symbolic_link);
    // Establece result a partir de attributes->SetString(.
    result = attributes->SetString(
        // Aporta MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_SYMBOLIC_LINK como elemento de la lista.
        MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_SYMBOLIC_LINK,
        // Ejecuta symbolic_link.c_str().
        symbolic_link.c_str()
    );

    // Evalúa la condición FAILED(result).
    if (FAILED(result)) {
        // Interrumpe la operación con HResultError(.
        throw HResultError(
            // Aporta result como elemento de la lista.
            result,
            // Aporta el texto "IMFAttributes::SetString(camera symbolic link)" a la expresión.
            "IMFAttributes::SetString(camera symbolic link)"
        );
    }

    // Declara media_source con el tipo indicado.
    ComPtr<IMFMediaSource> media_source;
    // Establece result a partir de MFCreateDeviceSource(attributes.Get(), &media_source).
    result = MFCreateDeviceSource(attributes.Get(), &media_source);

    // Evalúa la condición FAILED(result).
    if (FAILED(result)) {
        // Interrumpe la operación con HResultError(result, "MFCreateDeviceSource").
        throw HResultError(result, "MFCreateDeviceSource");
    }

    // Establece characteristics a partir de 0.
    DWORD characteristics = 0;
    // Establece result a partir de media_source->GetCharacteristics(&characteristics).
    result = media_source->GetCharacteristics(&characteristics);

    // Evalúa la condición FAILED(result).
    if (FAILED(result)) {
        // Ejecuta static_cast<void>(media_source->Shutdown()).
        static_cast<void>(media_source->Shutdown());
        // Interrumpe la operación con HResultError(result, "IMFMediaSource::GetCharacteristics").
        throw HResultError(result, "IMFMediaSource::GetCharacteristics");
    }

    // Declara source_reader con el tipo indicado.
    ComPtr<IMFSourceReader> source_reader;
    // Establece result a partir de MFCreateSourceReaderFromMediaSource(.
    result = MFCreateSourceReaderFromMediaSource(
        // Ejecuta media_source.Get(),.
        media_source.Get(),
        // Aporta nullptr como elemento de la lista.
        nullptr,
        // Incorpora &source_reader en esta operación.
        &source_reader
    );

    // Evalúa la condición FAILED(result).
    if (FAILED(result)) {
        // Ejecuta static_cast<void>(media_source->Shutdown()).
        static_cast<void>(media_source->Shutdown());
        // Interrumpe la operación con HResultError(result, "MFCreateSourceReaderFromMediaSource").
        throw HResultError(result, "MFCreateSourceReaderFromMediaSource");
    }

    // Establece result a partir de source_reader->SetStreamSelection(.
    result = source_reader->SetStreamSelection(
        // Aporta kAllStreams como elemento de la lista.
        kAllStreams,
        // Incorpora FALSE en esta operación.
        FALSE
    );

    // Evalúa la condición FAILED(result).
    if (FAILED(result)) {
        // Ejecuta source_reader.Reset().
        source_reader.Reset();
        // Ejecuta static_cast<void>(media_source->Shutdown()).
        static_cast<void>(media_source->Shutdown());
        // Interrumpe la operación con HResultError(result, "IMFSourceReader::SetStreamSelection(all)").
        throw HResultError(result, "IMFSourceReader::SetStreamSelection(all)");
    }

    // Establece result a partir de source_reader->SetStreamSelection(.
    result = source_reader->SetStreamSelection(
        // Aporta kFirstVideoStream como elemento de la lista.
        kFirstVideoStream,
        // Incorpora TRUE en esta operación.
        TRUE
    );

    // Evalúa la condición FAILED(result).
    if (FAILED(result)) {
        // Ejecuta source_reader.Reset().
        source_reader.Reset();
        // Ejecuta static_cast<void>(media_source->Shutdown()).
        static_cast<void>(media_source->Shutdown());
        // Interrumpe la operación con HResultError(result, "IMFSourceReader::SetStreamSelection(video)").
        throw HResultError(result, "IMFSourceReader::SetStreamSelection(video)");
    }

    // Establece impl_->media_source a partir de std::move(media_source).
    impl_->media_source = std::move(media_source);
    // Establece impl_->source_reader a partir de std::move(source_reader).
    impl_->source_reader = std::move(source_reader);
    // Establece impl_->format_configured a partir de false.
    impl_->format_configured = false;
}

// Define la operación MediaFoundationCamera::close con los parámetros declarados.
void MediaFoundationCamera::close() noexcept {
    // Evalúa la condición impl_ == nullptr.
    if (impl_ == nullptr) {
        // Termina la función sin un valor de retorno.
        return;
    }

    // Ejecuta impl_->source_reader.Reset().
    impl_->source_reader.Reset();
    // Establece impl_->configured_format a partir de {}.
    impl_->configured_format = {};
    // Establece impl_->format_configured a partir de false.
    impl_->format_configured = false;

    // Evalúa la condición impl_->media_source != nullptr.
    if (impl_->media_source != nullptr) {
        // Ejecuta static_cast<void>(impl_->media_source->Shutdown()).
        static_cast<void>(impl_->media_source->Shutdown());
        // Ejecuta impl_->media_source.Reset().
        impl_->media_source.Reset();
    }
}

// Define la operación MediaFoundationCamera::available_formats con los parámetros declarados.
std::vector<VideoFormat> MediaFoundationCamera::available_formats() const {
    // Evalúa la condición !is_open() || impl_->source_reader == nullptr.
    if (!is_open() || impl_->source_reader == nullptr) {
        // Interrumpe la operación con std::logic_error(.
        throw std::logic_error(
            // Aporta el texto "A camera must be open before enumerating video formats." a la expresión.
            "A camera must be open before enumerating video formats."
        );
    }

    // Declara formats con el tipo indicado.
    std::vector<VideoFormat> formats;

    // Recorre según DWORD index = 0;; ++index.
    for (DWORD index = 0;; ++index) {
        // Declara media_type con el tipo indicado.
        ComPtr<IMFMediaType> media_type;
        // Establece result a partir de impl_->source_reader->GetNativeMediaType(.
        const HRESULT result = impl_->source_reader->GetNativeMediaType(
            // Aporta kFirstVideoStream como elemento de la lista.
            kFirstVideoStream,
            // Aporta index como elemento de la lista.
            index,
            // Incorpora &media_type en esta operación.
            &media_type
        );

        // Evalúa la condición result == MF_E_NO_MORE_TYPES.
        if (result == MF_E_NO_MORE_TYPES) {
            // Termina el bucle o la rama actual.
            break;
        }

        // Evalúa la condición FAILED(result).
        if (FAILED(result)) {
            // Interrumpe la operación con HResultError(.
            throw HResultError(
                // Aporta result como elemento de la lista.
                result,
                // Aporta el texto "IMFSourceReader::GetNativeMediaType" a la expresión.
                "IMFSourceReader::GetNativeMediaType"
            );
        }

        // Declara format (formato de video) con el tipo indicado.
        VideoFormat format;
        // Evalúa la condición try_read_video_format(media_type.Get(), index, format).
        if (try_read_video_format(media_type.Get(), index, format)) {
            // Agrega un elemento al final del contenedor mediante formats.push_back.
            formats.push_back(std::move(format));
        }
    }

    // Devuelve formats.
    return formats;
}

// Define la operación MediaFoundationCamera::configure con los parámetros declarados.
VideoFormat MediaFoundationCamera::configure(const VideoFormatTarget& target) {
    // Evalúa la condición !is_open() || impl_->source_reader == nullptr.
    if (!is_open() || impl_->source_reader == nullptr) {
        // Interrumpe la operación con std::logic_error(.
        throw std::logic_error(
            // Aporta el texto "A camera must be open before configuring a video format." a la expresión.
            "A camera must be open before configuring a video format."
        );
    }

    // Establece impl_->format_configured a partir de false.
    impl_->format_configured = false;

    // Establece formats a partir de available_formats().
    const std::vector<VideoFormat> formats = available_formats();
    // Establece selected a partir de VideoFormatSelector::select_best(formats, target).
    const VideoFormat& selected =
        // Ejecuta VideoFormatSelector::select_best(formats, target).
        VideoFormatSelector::select_best(formats, target);

    // Declara media_type con el tipo indicado.
    ComPtr<IMFMediaType> media_type;
    // Establece result a partir de impl_->source_reader->GetNativeMediaType(.
    HRESULT result = impl_->source_reader->GetNativeMediaType(
        // Aporta kFirstVideoStream como elemento de la lista.
        kFirstVideoStream,
        // Aporta static_cast<DWORD>(selected.native_type_index) como elemento de la lista.
        static_cast<DWORD>(selected.native_type_index),
        // Incorpora &media_type en esta operación.
        &media_type
    );

    // Evalúa la condición FAILED(result).
    if (FAILED(result)) {
        // Interrumpe la operación con HResultError(.
        throw HResultError(
            // Aporta result como elemento de la lista.
            result,
            // Aporta el texto "IMFSourceReader::GetNativeMediaType(selected)" a la expresión.
            "IMFSourceReader::GetNativeMediaType(selected)"
        );
    }

    // Establece result a partir de impl_->source_reader->SetCurrentMediaType(.
    result = impl_->source_reader->SetCurrentMediaType(
        // Aporta kFirstVideoStream como elemento de la lista.
        kFirstVideoStream,
        // Aporta nullptr como elemento de la lista.
        nullptr,
        // Ejecuta media_type.Get().
        media_type.Get()
    );

    // Evalúa la condición FAILED(result).
    if (FAILED(result)) {
        // Interrumpe la operación con HResultError(result, "IMFSourceReader::SetCurrentMediaType").
        throw HResultError(result, "IMFSourceReader::SetCurrentMediaType");
    }

    // Declara configured_type con el tipo indicado.
    ComPtr<IMFMediaType> configured_type;
    // Establece result a partir de impl_->source_reader->GetCurrentMediaType(.
    result = impl_->source_reader->GetCurrentMediaType(
        // Aporta kFirstVideoStream como elemento de la lista.
        kFirstVideoStream,
        // Incorpora &configured_type en esta operación.
        &configured_type
    );

    // Evalúa la condición FAILED(result).
    if (FAILED(result)) {
        // Interrumpe la operación con HResultError(result, "IMFSourceReader::GetCurrentMediaType").
        throw HResultError(result, "IMFSourceReader::GetCurrentMediaType");
    }

    // Declara configured con el tipo indicado.
    VideoFormat configured;
    // Evalúa la condición !try_read_video_format(.
    if (!try_read_video_format(
        // Ejecuta configured_type.Get(),.
        configured_type.Get(),
        // Aporta selected.native_type_index como elemento de la lista.
        selected.native_type_index,
        // Incorpora configured en esta operación.
        configured
    )) {
        // Interrumpe la operación con std::runtime_error(.
        throw std::runtime_error(
            // Aporta el texto "Media Foundation returned an incomplete configured video format." a la expresión.
            "Media Foundation returned an incomplete configured video format."
        );
    }

    // Establece impl_->format_configured a partir de true.
    impl_->format_configured = true;
    // Establece impl_->configured_format a partir de configured.
    impl_->configured_format = configured;
    // Devuelve configured.
    return configured;
}

// Define la operación MediaFoundationCamera::capture_frames con los parámetros declarados.
CaptureSummary MediaFoundationCamera::capture_frames(
    // Declara el parámetro requested_frames.
    const std::size_t requested_frames,
    // Incorpora FrameSink* const sink en esta operación.
    FrameSink* const sink
) {
    // Evalúa la condición !is_open() || impl_->source_reader == nullptr.
    if (!is_open() || impl_->source_reader == nullptr) {
        // Interrumpe la operación con std::logic_error(.
        throw std::logic_error(
            // Aporta el texto "A camera must be open before capturing frames." a la expresión.
            "A camera must be open before capturing frames."
        );
    }

    // Evalúa la condición !impl_->format_configured.
    if (!impl_->format_configured) {
        // Interrumpe la operación con std::logic_error(.
        throw std::logic_error(
            // Aporta el texto "A video format must be configured before capturing frames." a la expresión.
            "A video format must be configured before capturing frames."
        );
    }

    // Define la operación reader con los parámetros declarados.
    MediaFoundationFrameReader reader(
        // Ejecuta impl_->source_reader.Get(),.
        impl_->source_reader.Get(),
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
        // Aporta 1000 como elemento de la lista.
        1000,
        // Incorpora sink en esta operación.
        sink
    );
}

// Define la operación MediaFoundationCamera::is_open con los parámetros declarados.
bool MediaFoundationCamera::is_open() const noexcept {
    // Devuelve impl_ != nullptr && impl_->media_source != nullptr.
    return impl_ != nullptr && impl_->media_source != nullptr;
}

// Incorpora } // namespace orvix::capture en esta operación.
}  // namespace orvix::capture
