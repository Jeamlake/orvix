#include "orvix/capture/media_foundation_runtime.hpp"

#include "orvix/capture/hresult_error.hpp"

#include <Windows.h>
#include <mfapi.h>

namespace orvix::capture {

ComRuntime::ComRuntime() {
    const HRESULT result = CoInitializeEx(nullptr, COINIT_MULTITHREADED);

    if (FAILED(result)) {
        throw HResultError(result, "CoInitializeEx");
    }

    initialized_ = true;
}

ComRuntime::~ComRuntime() {
    if (initialized_) {
        CoUninitialize();
    }
}

MediaFoundationRuntime::MediaFoundationRuntime() {
    const HRESULT result = MFStartup(MF_VERSION);

    if (FAILED(result)) {
        throw HResultError(result, "MFStartup");
    }

    initialized_ = true;
}

MediaFoundationRuntime::~MediaFoundationRuntime() {
    if (initialized_) {
        MFShutdown();
    }
}

}  // namespace orvix::capture
