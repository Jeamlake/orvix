#include "orvix/capture/av_foundation_device_enumerator.hpp"

#import <AVFoundation/AVFoundation.h>

#include <string>
#include <utility>
#include <vector>

namespace orvix::capture {

namespace {

std::string utf8_string(NSString* value) {
    if (value == nil) {
        return {};
    }

    const char* utf8 = value.UTF8String;
    return utf8 == nullptr ? std::string{} : std::string{utf8};
}

}  // namespace

std::vector<CameraDevice> AvFoundationDeviceEnumerator::enumerate() const {
    @autoreleasepool {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
        NSArray<AVCaptureDevice*>* capture_devices =
            [AVCaptureDevice devicesWithMediaType:AVMediaTypeVideo];
#pragma clang diagnostic pop

        std::vector<CameraDevice> devices;
        devices.reserve(capture_devices.count);

        for (AVCaptureDevice* device in capture_devices) {
            devices.push_back({
                devices.size(),
                utf8_string(device.localizedName),
                utf8_string(device.uniqueID),
                "macOS AVFoundation"
            });
        }

        return devices;
    }
}

}  // namespace orvix::capture
