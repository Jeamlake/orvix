// Incluye las declaraciones de media foundation runtime de ORVIX.
#include "orvix/capture/media_foundation_runtime.hpp"

// Incluye las declaraciones de hresult error de ORVIX.
#include "orvix/capture/hresult_error.hpp"

// Importa las declaraciones de Windows.h para esta unidad.
#include <Windows.h>
// Importa las declaraciones de mfapi.h para esta unidad.
#include <mfapi.h>

// Agrupa las declaraciones bajo orvix::capture.
namespace orvix::capture {

// Define la operación e con los parámetros declarados.
ComRuntime::ComRuntime() {
    // Establece result a partir de CoInitializeEx(nullptr, COINIT_MULTITHREADED).
    const HRESULT result = CoInitializeEx(nullptr, COINIT_MULTITHREADED);

    // Evalúa la condición FAILED(result).
    if (FAILED(result)) {
        // Interrumpe la operación con HResultError(result, "CoInitializeEx").
        throw HResultError(result, "CoInitializeEx");
    }

    // Establece initialized_ a partir de true.
    initialized_ = true;
}

// Ejecuta ComRuntime::~ComRuntime() {.
ComRuntime::~ComRuntime() {
    // Evalúa la condición initialized_.
    if (initialized_) {
        // Cierra COM para el hilo mediante CoUninitialize.
        CoUninitialize();
    }
}

// Define la operación e con los parámetros declarados.
MediaFoundationRuntime::MediaFoundationRuntime() {
    // Establece result a partir de MFStartup(MF_VERSION).
    const HRESULT result = MFStartup(MF_VERSION);

    // Evalúa la condición FAILED(result).
    if (FAILED(result)) {
        // Interrumpe la operación con HResultError(result, "MFStartup").
        throw HResultError(result, "MFStartup");
    }

    // Establece initialized_ a partir de true.
    initialized_ = true;
}

// Ejecuta MediaFoundationRuntime::~MediaFoundationRuntime() {.
MediaFoundationRuntime::~MediaFoundationRuntime() {
    // Evalúa la condición initialized_.
    if (initialized_) {
        // Cierra Media Foundation mediante MFShutdown.
        MFShutdown();
    }
}

// Incorpora } // namespace orvix::capture en esta operación.
}  // namespace orvix::capture
