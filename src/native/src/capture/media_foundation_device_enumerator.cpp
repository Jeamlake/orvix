// Incluye las declaraciones de media foundation device enumerator de ORVIX.
#include "orvix/capture/media_foundation_device_enumerator.hpp"

// Incluye las declaraciones de hresult error de ORVIX.
#include "orvix/capture/hresult_error.hpp"
// Incluye las declaraciones de media foundation runtime de ORVIX.
#include "orvix/capture/media_foundation_runtime.hpp"

// Importa las declaraciones de Windows.h para esta unidad.
#include <Windows.h>
// Importa las declaraciones de mfapi.h para esta unidad.
#include <mfapi.h>
// Importa las declaraciones de mfidl.h para esta unidad.
#include <mfidl.h>
// Importa las declaraciones de wrl/client.h para esta unidad.
#include <wrl/client.h>

// Incluye cstddef para usar tipos de tamaño y bytes.
#include <cstddef>
// Incluye memory para usar punteros inteligentes y gestión de memoria.
#include <memory>
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

// Declara el tipo CoTaskMemString y las operaciones o datos de su contrato.
class CoTaskMemString final {
// Expone los miembros de la interfaz.
public:
    // Solicita la implementación predeterminada del compilador.
    CoTaskMemString() = default;

    // Define ~CoTaskMemString para cerrar los recursos al destruir la instancia.
    ~CoTaskMemString() {
        // Reinicia el recurso o estado mediante reset.
        reset();
    }

    // Prohíbe esta operación en tiempo de compilación.
    CoTaskMemString(const CoTaskMemString&) = delete;
    // Establece operator a partir de (const CoTaskMemString&) = delete.
    CoTaskMemString& operator=(const CoTaskMemString&) = delete;

    // Declara el atributo de compilación [[nodiscard]].
    [[nodiscard]]
    // Define la operación put con los parámetros declarados.
    wchar_t** put() noexcept {
        // Reinicia el recurso o estado mediante reset.
        reset();
        // Devuelve &value_.
        return &value_;
    }

    // Declara el atributo de compilación [[nodiscard]].
    [[nodiscard]]
    // Define la operación get con los parámetros declarados.
    const wchar_t* get() const noexcept {
        // Devuelve value_.
        return value_;
    }

// Restringe estos miembros a la implementación.
private:
    // Define la operación reset, que reinicia el recurso o estado.
    void reset() noexcept {
        // Evalúa la condición value_ != nullptr.
        if (value_ != nullptr) {
            // Ejecuta CoTaskMemFree(value_).
            CoTaskMemFree(value_);
            // Establece value_ a partir de nullptr.
            value_ = nullptr;
        }
    }

    // Inicializa value_ con nullptr.
    wchar_t* value_{nullptr};
};

// Declara el tipo ActivateArray y las operaciones o datos de su contrato.
class ActivateArray final {
// Expone los miembros de la interfaz.
public:
    // Ejecuta ActivateArray(IMFActivate** values, const UINT32 count).
    ActivateArray(IMFActivate** values, const UINT32 count)
        // Inicializa el miembro indicado por values_(values), antes del cuerpo.
        : values_(values),
          // Ejecuta count_(count) {}.
          count_(count) {}

    // Define ~ActivateArray para cerrar los recursos al destruir la instancia.
    ~ActivateArray() {
        // Evalúa la condición values_ != nullptr.
        if (values_ != nullptr) {
            // Recorre según UINT32 i = 0; i < count_; ++i.
            for (UINT32 i = 0; i < count_; ++i) {
                // Evalúa la condición values_[i] != nullptr.
                if (values_[i] != nullptr) {
                    // Ejecuta values_[i]->Release().
                    values_[i]->Release();
                }
            }

            // Ejecuta CoTaskMemFree(values_).
            CoTaskMemFree(values_);
        }
    }

    // Prohíbe esta operación en tiempo de compilación.
    ActivateArray(const ActivateArray&) = delete;
    // Establece operator a partir de (const ActivateArray&) = delete.
    ActivateArray& operator=(const ActivateArray&) = delete;

    // Declara el atributo de compilación [[nodiscard]].
    [[nodiscard]]
    // Incorpora IMFActivate* operator[](const UINT32 index) const noexcept { en esta operación.
    IMFActivate* operator[](const UINT32 index) const noexcept {
        // Devuelve values_[index].
        return values_[index];
    }

// Restringe estos miembros a la implementación.
private:
    // Inicializa values_ con nullptr.
    IMFActivate** values_{nullptr};
    // Inicializa count_ con 0.
    UINT32 count_{0};
};

// Define la operación wide_to_utf8 con los parámetros declarados.
std::string wide_to_utf8(const wchar_t* value) {
    // Evalúa la condición value == nullptr || value[0] == L'\0'.
    if (value == nullptr || value[0] == L'\0') {
        // Devuelve {}.
        return {};
    }

    // Establece required a partir de WideCharToMultiByte(.
    const int required = WideCharToMultiByte(
        // Aporta CP_UTF8 como elemento de la lista.
        CP_UTF8,
        // Aporta WC_ERR_INVALID_CHARS como elemento de la lista.
        WC_ERR_INVALID_CHARS,
        // Aporta value como elemento de la lista.
        value,
        // Aporta -1 como elemento de la lista.
        -1,
        // Aporta nullptr como elemento de la lista.
        nullptr,
        // Aporta 0 como elemento de la lista.
        0,
        // Aporta nullptr como elemento de la lista.
        nullptr,
        // Incorpora nullptr en esta operación.
        nullptr
    );

    // Evalúa la condición required <= 0.
    if (required <= 0) {
        // Interrumpe la operación con HResultError(.
        throw HResultError(
            // Ejecuta HRESULT_FROM_WIN32(GetLastError()),.
            HRESULT_FROM_WIN32(GetLastError()),
            // Aporta el texto "WideCharToMultiByte(size)" a la expresión.
            "WideCharToMultiByte(size)"
        );
    }

    // Ejecuta std::string utf8(static_cast<std::size_t>(required), '\0').
    std::string utf8(static_cast<std::size_t>(required), '\0');

    // Establece written a partir de WideCharToMultiByte(.
    const int written = WideCharToMultiByte(
        // Aporta CP_UTF8 como elemento de la lista.
        CP_UTF8,
        // Aporta WC_ERR_INVALID_CHARS como elemento de la lista.
        WC_ERR_INVALID_CHARS,
        // Aporta value como elemento de la lista.
        value,
        // Aporta -1 como elemento de la lista.
        -1,
        // Ejecuta utf8.data(),.
        utf8.data(),
        // Aporta required como elemento de la lista.
        required,
        // Aporta nullptr como elemento de la lista.
        nullptr,
        // Incorpora nullptr en esta operación.
        nullptr
    );

    // Evalúa la condición written <= 0.
    if (written <= 0) {
        // Interrumpe la operación con HResultError(.
        throw HResultError(
            // Ejecuta HRESULT_FROM_WIN32(GetLastError()),.
            HRESULT_FROM_WIN32(GetLastError()),
            // Aporta el texto "WideCharToMultiByte(convert)" a la expresión.
            "WideCharToMultiByte(convert)"
        );
    }

    // Evalúa la condición !utf8.empty() && utf8.back() == '\0'.
    if (!utf8.empty() && utf8.back() == '\0') {
        // Ejecuta utf8.pop_back().
        utf8.pop_back();
    }

    // Devuelve utf8.
    return utf8;
}

// Define la operación get_string_attribute con los parámetros declarados.
std::string get_string_attribute(
    // Declara el parámetro activate.
    IMFActivate* activate,
    // Declara el parámetro attribute.
    const GUID& attribute,
    // Incorpora const char* operation en esta operación.
    const char* operation
) {
    // Declara value con el tipo indicado.
    CoTaskMemString value;
    // Establece length a partir de 0.
    UINT32 length = 0;

    // Establece result a partir de activate->GetAllocatedString(.
    const HRESULT result = activate->GetAllocatedString(
        // Aporta attribute como elemento de la lista.
        attribute,
        // Ejecuta value.put(),.
        value.put(),
        // Incorpora &length en esta operación.
        &length
    );

    // Evalúa la condición FAILED(result).
    if (FAILED(result)) {
        // Interrumpe la operación con HResultError(result, operation).
        throw HResultError(result, operation);
    }

    // Devuelve wide_to_utf8(value.get()).
    return wide_to_utf8(value.get());
}

// Incorpora } // namespace en esta operación.
}  // namespace

// Incorpora std::vector<CameraDevice> en esta operación.
std::vector<CameraDevice>
// Define la operación e con los parámetros declarados.
MediaFoundationDeviceEnumerator::enumerate() const {
    // Declara com_runtime con el tipo indicado.
    const ComRuntime com_runtime;
    // Declara media_foundation_runtime con el tipo indicado.
    const MediaFoundationRuntime media_foundation_runtime;

    // Declara attributes con el tipo indicado.
    ComPtr<IMFAttributes> attributes;

    // Establece result a partir de MFCreateAttributes(&attributes, 1).
    HRESULT result = MFCreateAttributes(&attributes, 1);

    // Evalúa la condición FAILED(result).
    if (FAILED(result)) {
        // Interrumpe la operación con HResultError(result, "MFCreateAttributes").
        throw HResultError(result, "MFCreateAttributes");
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

    // Establece raw_devices a partir de nullptr.
    IMFActivate** raw_devices = nullptr;
    // Establece device_count a partir de 0.
    UINT32 device_count = 0;

    // Establece result a partir de MFEnumDeviceSources(.
    result = MFEnumDeviceSources(
        // Ejecuta attributes.Get(),.
        attributes.Get(),
        // Aporta &raw_devices como elemento de la lista.
        &raw_devices,
        // Incorpora &device_count en esta operación.
        &device_count
    );

    // Evalúa la condición FAILED(result).
    if (FAILED(result)) {
        // Interrumpe la operación con HResultError(result, "MFEnumDeviceSources").
        throw HResultError(result, "MFEnumDeviceSources");
    }

    // Ejecuta const ActivateArray devices(raw_devices, device_count).
    const ActivateArray devices(raw_devices, device_count);

    // Declara result_devices con el tipo indicado.
    std::vector<CameraDevice> result_devices;
    // Reserva capacidad en el contenedor mediante result_devices.reserve.
    result_devices.reserve(device_count);

    // Recorre según UINT32 i = 0; i < device_count; ++i.
    for (UINT32 i = 0; i < device_count; ++i) {
        // Establece activate a partir de devices[i].
        IMFActivate* activate = devices[i];

        // Declara device (dispositivo) con el tipo indicado.
        CameraDevice device;
        // Establece device.index a partir de static_cast<std::size_t>(i).
        device.index = static_cast<std::size_t>(i);
        // Establece device.friendly_name a partir de get_string_attribute(.
        device.friendly_name = get_string_attribute(
            // Aporta activate como elemento de la lista.
            activate,
            // Aporta MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME como elemento de la lista.
            MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME,
            // Aporta el texto "GetAllocatedString(FRIENDLY_NAME)" a la expresión.
            "GetAllocatedString(FRIENDLY_NAME)"
        );
        // Establece device.symbolic_link a partir de get_string_attribute(.
        device.symbolic_link = get_string_attribute(
            // Aporta activate como elemento de la lista.
            activate,
            // Aporta MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_SYMBOLIC_LINK como elemento de la lista.
            MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_SYMBOLIC_LINK,
            // Aporta el texto "GetAllocatedString(SYMBOLIC_LINK)" a la expresión.
            "GetAllocatedString(SYMBOLIC_LINK)"
        );
        // Establece device.backend (implementación nativa) a partir de "Windows Media Foundation".
        device.backend = "Windows Media Foundation";

        // Agrega un elemento al final del contenedor mediante result_devices.push_back.
        result_devices.push_back(std::move(device));
    }

    // Devuelve result_devices.
    return result_devices;
}

// Incorpora } // namespace orvix::capture en esta operación.
}  // namespace orvix::capture
