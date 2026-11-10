// Incluye las declaraciones de av foundation device enumerator de ORVIX.
#include "orvix/capture/av_foundation_device_enumerator.hpp"

// Importa las declaraciones de AVFoundation/AVFoundation.h para esta unidad.
#import <AVFoundation/AVFoundation.h>

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

// Incorpora } // namespace en esta operación.
}  // namespace

// Define la operación AvFoundationDeviceEnumerator::enumerate con los parámetros declarados.
std::vector<CameraDevice> AvFoundationDeviceEnumerator::enumerate() const {
    // Delimita el grupo de liberación automática de objetos.
    @autoreleasepool {
// Aplica la directiva del compilador clang diagnostic push.
#pragma clang diagnostic push
// Aplica la directiva del compilador clang diagnostic ignored "-Wdeprecated-declarations".
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
        // Establece capture_devices a partir de [AVCaptureDevice devicesWithMediaType:AVMediaTypeVideo].
        NSArray<AVCaptureDevice*>* capture_devices =
            // Envía el mensaje de Objective-C indicado por [AVCaptureDevice devicesWithMediaType:AVMediaTypeVideo].
            [AVCaptureDevice devicesWithMediaType:AVMediaTypeVideo];
// Aplica la directiva del compilador clang diagnostic pop.
#pragma clang diagnostic pop

        // Declara devices (dispositivos detectados) con el tipo indicado.
        std::vector<CameraDevice> devices;
        // Reserva capacidad en el contenedor mediante devices.reserve.
        devices.reserve(capture_devices.count);

        // Recorre según AVCaptureDevice* device in capture_devices.
        for (AVCaptureDevice* device in capture_devices) {
            // Agrega un elemento al final del contenedor mediante devices.push_back.
            devices.push_back({
                // Ejecuta devices.size(),.
                devices.size(),
                // Ejecuta utf8_string(device.localizedName),.
                utf8_string(device.localizedName),
                // Ejecuta utf8_string(device.uniqueID),.
                utf8_string(device.uniqueID),
                // Aporta el texto "macOS AVFoundation" a la expresión.
                "macOS AVFoundation"
            });
        }

        // Devuelve devices.
        return devices;
    }
}

// Incorpora } // namespace orvix::capture en esta operación.
}  // namespace orvix::capture
