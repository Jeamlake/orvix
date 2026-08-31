#include "orvix/capture/media_foundation_device_enumerator.hpp"

#include "orvix/capture/hresult_error.hpp"

#include <Windows.h>
#include <mfapi.h>
#include <mfidl.h>
#include <wrl/client.h>

#include <cstddef>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace orvix::capture {

namespace {

using Microsoft::WRL::ComPtr;

class ComRuntime final {
public:
    ComRuntime() {
        const HRESULT result = CoInitializeEx(nullptr, COINIT_MULTITHREADED);

        if (FAILED(result)) {
            throw HResultError(result, "CoInitializeEx");
        }

        initialized_ = true;
    }

    ~ComRuntime() {
        if (initialized_) {
            CoUninitialize();
        }
    }

    ComRuntime(const ComRuntime&) = delete;
    ComRuntime& operator=(const ComRuntime&) = delete;
    ComRuntime(ComRuntime&&) = delete;
    ComRuntime& operator=(ComRuntime&&) = delete;

private:
    bool initialized_{false};
};

class MediaFoundationRuntime final {
public:
    MediaFoundationRuntime() {
        const HRESULT result = MFStartup(MF_VERSION);

        if (FAILED(result)) {
            throw HResultError(result, "MFStartup");
        }

        initialized_ = true;
    }

    ~MediaFoundationRuntime() {
        if (initialized_) {
            MFShutdown();
        }
    }

    MediaFoundationRuntime(const MediaFoundationRuntime&) = delete;
    MediaFoundationRuntime& operator=(const MediaFoundationRuntime&) = delete;
    MediaFoundationRuntime(MediaFoundationRuntime&&) = delete;
    MediaFoundationRuntime& operator=(MediaFoundationRuntime&&) = delete;

private:
    bool initialized_{false};
};

class CoTaskMemString final {
public:
    CoTaskMemString() = default;

    ~CoTaskMemString() {
        reset();
    }

    CoTaskMemString(const CoTaskMemString&) = delete;
    CoTaskMemString& operator=(const CoTaskMemString&) = delete;

    [[nodiscard]]
    wchar_t** put() noexcept {
        reset();
        return &value_;
    }

    [[nodiscard]]
    const wchar_t* get() const noexcept {
        return value_;
    }

private:
    void reset() noexcept {
        if (value_ != nullptr) {
            CoTaskMemFree(value_);
            value_ = nullptr;
        }
    }

    wchar_t* value_{nullptr};
};

class ActivateArray final {
public:
    ActivateArray(IMFActivate** values, const UINT32 count)
        : values_(values),
          count_(count) {}

    ~ActivateArray() {
        if (values_ != nullptr) {
            for (UINT32 i = 0; i < count_; ++i) {
                if (values_[i] != nullptr) {
                    values_[i]->Release();
                }
            }

            CoTaskMemFree(values_);
        }
    }

    ActivateArray(const ActivateArray&) = delete;
    ActivateArray& operator=(const ActivateArray&) = delete;

    [[nodiscard]]
    IMFActivate* operator[](const UINT32 index) const noexcept {
        return values_[index];
    }

private:
    IMFActivate** values_{nullptr};
    UINT32 count_{0};
};

std::string wide_to_utf8(const wchar_t* value) {
    if (value == nullptr || value[0] == L'\0') {
        return {};
    }

    const int required = WideCharToMultiByte(
        CP_UTF8,
        WC_ERR_INVALID_CHARS,
        value,
        -1,
        nullptr,
        0,
        nullptr,
        nullptr
    );

    if (required <= 0) {
        throw HResultError(
            HRESULT_FROM_WIN32(GetLastError()),
            "WideCharToMultiByte(size)"
        );
    }

    std::string utf8(static_cast<std::size_t>(required), '\0');

    const int written = WideCharToMultiByte(
        CP_UTF8,
        WC_ERR_INVALID_CHARS,
        value,
        -1,
        utf8.data(),
        required,
        nullptr,
        nullptr
    );

    if (written <= 0) {
        throw HResultError(
            HRESULT_FROM_WIN32(GetLastError()),
            "WideCharToMultiByte(convert)"
        );
    }

    if (!utf8.empty() && utf8.back() == '\0') {
        utf8.pop_back();
    }

    return utf8;
}

std::string get_string_attribute(
    IMFActivate* activate,
    const GUID& attribute,
    const char* operation
) {
    CoTaskMemString value;
    UINT32 length = 0;

    const HRESULT result = activate->GetAllocatedString(
        attribute,
        value.put(),
        &length
    );

    if (FAILED(result)) {
        throw HResultError(result, operation);
    }

    return wide_to_utf8(value.get());
}

}  // namespace

std::vector<CameraDevice>
MediaFoundationDeviceEnumerator::enumerate() const {
    const ComRuntime com_runtime;
    const MediaFoundationRuntime media_foundation_runtime;

    ComPtr<IMFAttributes> attributes;

    HRESULT result = MFCreateAttributes(&attributes, 1);

    if (FAILED(result)) {
        throw HResultError(result, "MFCreateAttributes");
    }

    result = attributes->SetGUID(
        MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE,
        MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID
    );

    if (FAILED(result)) {
        throw HResultError(
            result,
            "IMFAttributes::SetGUID(video capture)"
        );
    }

    IMFActivate** raw_devices = nullptr;
    UINT32 device_count = 0;

    result = MFEnumDeviceSources(
        attributes.Get(),
        &raw_devices,
        &device_count
    );

    if (FAILED(result)) {
        throw HResultError(result, "MFEnumDeviceSources");
    }

    const ActivateArray devices(raw_devices, device_count);

    std::vector<CameraDevice> result_devices;
    result_devices.reserve(device_count);

    for (UINT32 i = 0; i < device_count; ++i) {
        IMFActivate* activate = devices[i];

        CameraDevice device;
        device.index = static_cast<std::size_t>(i);
        device.friendly_name = get_string_attribute(
            activate,
            MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME,
            "GetAllocatedString(FRIENDLY_NAME)"
        );
        device.symbolic_link = get_string_attribute(
            activate,
            MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_SYMBOLIC_LINK,
            "GetAllocatedString(SYMBOLIC_LINK)"
        );
        device.backend = "Windows Media Foundation";

        result_devices.push_back(std::move(device));
    }

    return result_devices;
}

}  // namespace orvix::capture