#include "orvix/capture/av_foundation_camera.hpp"

#include "orvix/capture/continuous_frame_capture.hpp"
#include "orvix/capture/diagnostic_error.hpp"
#include "orvix/capture/video_format_selector.hpp"

#import <AVFoundation/AVFoundation.h>
#import <CoreMedia/CoreMedia.h>
#import <CoreVideo/CoreVideo.h>
#import <Foundation/Foundation.h>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <memory>
#include <mutex>
#include <numeric>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace orvix::capture::detail {

struct SampleQueue final {
    std::mutex mutex;
    std::condition_variable ready;
    std::deque<orvix::capture::FrameReadResult> frames;
    bool callback_failed{false};
};

}  // namespace orvix::capture::detail

@interface OrvixSampleBufferDelegate
    : NSObject <AVCaptureVideoDataOutputSampleBufferDelegate>

@property(nonatomic, assign) void* sampleQueueContext;

@end

@implementation OrvixSampleBufferDelegate

- (void)captureOutput:(AVCaptureOutput*)output
    didOutputSampleBuffer:(CMSampleBufferRef)sampleBuffer
    fromConnection:(AVCaptureConnection*)connection {
    static_cast<void>(output);
    static_cast<void>(connection);

    auto* queue = static_cast<orvix::capture::detail::SampleQueue*>(
        self.sampleQueueContext
    );
    if (queue == nullptr) {
        return;
    }

    try {
        const CMTime timestamp =
            CMSampleBufferGetPresentationTimeStamp(sampleBuffer);
        const CMTime timestamp_100ns = CMTimeConvertScale(
            timestamp,
            10'000'000,
            kCMTimeRoundingMethod_Default
        );

        const std::int64_t timestamp_value =
            CMTIME_IS_NUMERIC(timestamp_100ns)
                ? timestamp_100ns.value
                : 0;
        const std::size_t byte_count =
            CMSampleBufferGetTotalSampleSize(sampleBuffer);

        {
            const std::lock_guard lock(queue->mutex);
            queue->frames.push_back({true, timestamp_value, byte_count});
        }

        queue->ready.notify_one();
    }
    catch (...) {
        {
            const std::lock_guard lock(queue->mutex);
            queue->callback_failed = true;
        }
        queue->ready.notify_one();
    }
}

@end

namespace orvix::capture {

namespace {

using detail::SampleQueue;

struct NativeFormat final {
    VideoFormat format;
    __strong AVCaptureDeviceFormat* native_format{nil};
    CMTime frame_duration{kCMTimeInvalid};
};

std::string utf8_string(NSString* value) {
    if (value == nil) {
        return {};
    }

    const char* utf8 = value.UTF8String;
    return utf8 == nullptr ? std::string{} : std::string{utf8};
}

NSString* native_string(const std::string& value) {
    return [NSString stringWithUTF8String:value.c_str()];
}

std::string fourcc_string(const FourCharCode value) {
    std::string result(4, ' ');
    result[0] = static_cast<char>((value >> 24U) & 0xffU);
    result[1] = static_cast<char>((value >> 16U) & 0xffU);
    result[2] = static_cast<char>((value >> 8U) & 0xffU);
    result[3] = static_cast<char>(value & 0xffU);
    return result;
}

std::string pixel_format_name(const FourCharCode value) {
    switch (value) {
    case kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange:
    case kCVPixelFormatType_420YpCbCr8BiPlanarFullRange:
        return "NV12";
    case kCVPixelFormatType_422YpCbCr8_yuvs:
        return "YUY2";
    case kCVPixelFormatType_422YpCbCr8:
        return "UYVY";
    case kCVPixelFormatType_24RGB:
        return "RGB24";
    case kCVPixelFormatType_32BGRA:
        return "BGRA32";
    default:
        return fourcc_string(value);
    }
}

bool compressed_format(const FourCharCode value) noexcept {
    constexpr auto make_fourcc = [](
        const char first,
        const char second,
        const char third,
        const char fourth
    ) constexpr {
        return
            (static_cast<FourCharCode>(first) << 24U) |
            (static_cast<FourCharCode>(second) << 16U) |
            (static_cast<FourCharCode>(third) << 8U) |
            static_cast<FourCharCode>(fourth);
    };

    return
        value == make_fourcc('j', 'p', 'e', 'g') ||
        value == make_fourcc('m', 'j', 'p', 'a') ||
        value == make_fourcc('m', 'j', 'p', 'b') ||
        value == make_fourcc('a', 'v', 'c', '1') ||
        value == make_fourcc('h', 'v', 'c', '1');
}

std::pair<std::uint32_t, std::uint32_t> frame_rate_ratio(
    const double frame_rate
) {
    const auto scaled = static_cast<std::uint32_t>(
        std::llround(frame_rate * 1000.0)
    );
    const std::uint32_t divisor = std::gcd(scaled, 1000U);
    return {scaled / divisor, 1000U / divisor};
}

std::vector<NativeFormat> enumerate_formats(
    AVCaptureDevice* device,
    const VideoFormatTarget& target
) {
    std::vector<NativeFormat> formats;

    for (AVCaptureDeviceFormat* native_format in device.formats) {
        const CMFormatDescriptionRef description =
            native_format.formatDescription;
        const CMVideoDimensions dimensions =
            CMVideoFormatDescriptionGetDimensions(description);
        const FourCharCode subtype =
            CMFormatDescriptionGetMediaSubType(description);

        for (AVFrameRateRange* range in native_format.videoSupportedFrameRateRanges) {
            const double selected_rate = std::clamp(
                static_cast<double>(target.frames_per_second),
                range.minFrameRate,
                range.maxFrameRate
            );

            if (selected_rate <= 0.0) {
                continue;
            }

            const auto [numerator, denominator] =
                frame_rate_ratio(selected_rate);
            const CMTime frame_duration = CMTimeMake(
                static_cast<std::int64_t>(denominator),
                static_cast<std::int32_t>(numerator)
            );

            formats.push_back({
                {
                    formats.size(),
                    static_cast<std::uint32_t>(dimensions.width),
                    static_cast<std::uint32_t>(dimensions.height),
                    numerator,
                    denominator,
                    pixel_format_name(subtype),
                    compressed_format(subtype)
                },
                native_format,
                frame_duration
            });
        }
    }

    return formats;
}

void require_camera_permission() {
    AVAuthorizationStatus status =
        [AVCaptureDevice authorizationStatusForMediaType:AVMediaTypeVideo];

    if (status == AVAuthorizationStatusNotDetermined) {
        dispatch_semaphore_t permission_ready = dispatch_semaphore_create(0);
        __block BOOL granted = NO;

        [AVCaptureDevice requestAccessForMediaType:AVMediaTypeVideo
            completionHandler:^(BOOL access_granted) {
                granted = access_granted;
                dispatch_semaphore_signal(permission_ready);
            }];

        const dispatch_time_t timeout = dispatch_time(
            DISPATCH_TIME_NOW,
            static_cast<std::int64_t>(30) * NSEC_PER_SEC
        );

        if (dispatch_semaphore_wait(permission_ready, timeout) != 0) {
            throw DiagnosticError(
                "ORV-CAP-501",
                "camera_permission_timeout",
                "macOS did not return a camera permission decision.",
                71
            );
        }

        status = granted
            ? AVAuthorizationStatusAuthorized
            : AVAuthorizationStatusDenied;
    }

    if (status != AVAuthorizationStatusAuthorized) {
        throw DiagnosticError(
            "ORV-CAP-501",
            "camera_permission_denied",
            "Camera access is disabled in macOS Privacy & Security settings.",
            71
        );
    }
}

AVCaptureDevice* find_device(const std::string& unique_id) {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    NSArray<AVCaptureDevice*>* devices =
        [AVCaptureDevice devicesWithMediaType:AVMediaTypeVideo];
#pragma clang diagnostic pop

    NSString* requested_id = native_string(unique_id);
    for (AVCaptureDevice* device in devices) {
        if ([device.uniqueID isEqualToString:requested_id]) {
            return device;
        }
    }

    return nil;
}

class AvFoundationFrameReader final : public FrameReader {
public:
    AvFoundationFrameReader(
        SampleQueue& queue,
        AVCaptureDevice* device
    )
        : queue_(queue),
          device_(device) {}

    FrameReadResult read_next() override {
        std::unique_lock lock(queue_.mutex);
        const bool ready = queue_.ready.wait_for(
            lock,
            std::chrono::milliseconds(250),
            [this] {
                return !queue_.frames.empty() || queue_.callback_failed;
            }
        );

        if (!ready) {
            if (!device_.connected) {
                throw DiagnosticError(
                    "ORV-CAP-503",
                    "camera_disconnected",
                    "The AVFoundation camera was disconnected during capture.",
                    74
                );
            }
            return {};
        }

        if (queue_.callback_failed) {
            throw DiagnosticError(
                "ORV-CAP-504",
                "frame_read_failed",
                "The AVFoundation sample callback failed.",
                75
            );
        }

        const FrameReadResult frame = queue_.frames.front();
        queue_.frames.pop_front();
        return frame;
    }

private:
    SampleQueue& queue_;
    __strong AVCaptureDevice* device_;
};

}  // namespace

struct AvFoundationCamera::Impl final {
    __strong AVCaptureDevice* device{nil};
    __strong AVCaptureSession* session{nil};
    __strong AVCaptureVideoDataOutput* output{nil};
    __strong OrvixSampleBufferDelegate* delegate{nil};
    dispatch_queue_t callback_queue{nullptr};
    std::unique_ptr<SampleQueue> samples;
    VideoFormat configured_format;
    bool format_configured{false};
};

AvFoundationCamera::AvFoundationCamera()
    : impl_(std::make_unique<Impl>()) {}

AvFoundationCamera::~AvFoundationCamera() {
    close();
}

void AvFoundationCamera::open(const CameraDevice& device) {
    @autoreleasepool {
        if (is_open()) {
            throw std::logic_error("A camera is already open.");
        }

        if (device.symbolic_link.empty()) {
            throw std::invalid_argument(
                "The selected camera does not provide a unique identifier."
            );
        }

        require_camera_permission();

        AVCaptureDevice* capture_device = find_device(device.symbolic_link);
        if (capture_device == nil) {
            throw DiagnosticError(
                "ORV-CAP-501",
                "camera_open_failed",
                "The selected AVFoundation camera is no longer available.",
                71
            );
        }

        NSError* input_error = nil;
        AVCaptureDeviceInput* input =
            [AVCaptureDeviceInput deviceInputWithDevice:capture_device
                                                   error:&input_error];

        if (input == nil) {
            throw std::runtime_error(
                "AVCaptureDeviceInput failed: " +
                utf8_string(input_error.localizedDescription)
            );
        }

        AVCaptureSession* session = [[AVCaptureSession alloc] init];
        if (![session canAddInput:input]) {
            throw std::runtime_error(
                "AVFoundation rejected the selected camera input."
            );
        }

        [session addInput:input];
        impl_->device = capture_device;
        impl_->session = session;
    }
}

void AvFoundationCamera::close() noexcept {
    @autoreleasepool {
        if (impl_ == nullptr) {
            return;
        }

        if (impl_->session.running) {
            [impl_->session stopRunning];
        }

        [impl_->output setSampleBufferDelegate:nil queue:nullptr];
        impl_->delegate.sampleQueueContext = nullptr;

        if (impl_->callback_queue != nullptr) {
            dispatch_sync(impl_->callback_queue, ^{});
        }
        impl_->delegate = nil;
        impl_->output = nil;
        impl_->session = nil;
        impl_->device = nil;
        impl_->callback_queue = nullptr;
        impl_->samples.reset();
        impl_->configured_format = {};
        impl_->format_configured = false;
    }
}

std::vector<VideoFormat> AvFoundationCamera::available_formats() const {
    @autoreleasepool {
        if (!is_open()) {
            throw std::logic_error(
                "A camera must be open before enumerating video formats."
            );
        }

        const auto native_formats = enumerate_formats(
            impl_->device,
            VideoFormatTarget{}
        );
        std::vector<VideoFormat> formats;
        formats.reserve(native_formats.size());

        for (const auto& native_format : native_formats) {
            formats.push_back(native_format.format);
        }

        return formats;
    }
}

VideoFormat AvFoundationCamera::configure(const VideoFormatTarget& target) {
    @autoreleasepool {
        if (!is_open()) {
            throw std::logic_error(
                "A camera must be open before configuring a video format."
            );
        }

        const auto native_formats = enumerate_formats(impl_->device, target);
        std::vector<VideoFormat> formats;
        formats.reserve(native_formats.size());

        for (const auto& native_format : native_formats) {
            formats.push_back(native_format.format);
        }

        const VideoFormat& selected = VideoFormatSelector::select_best(
            formats,
            target
        );
        const NativeFormat& native = native_formats.at(
            selected.native_type_index
        );

        NSError* configuration_error = nil;
        if (![impl_->device lockForConfiguration:&configuration_error]) {
            throw std::runtime_error(
                "AVCaptureDevice::lockForConfiguration failed: " +
                utf8_string(configuration_error.localizedDescription)
            );
        }

        impl_->device.activeFormat = native.native_format;
        impl_->device.activeVideoMinFrameDuration = native.frame_duration;
        impl_->device.activeVideoMaxFrameDuration = native.frame_duration;
        [impl_->device unlockForConfiguration];

        auto samples = std::make_unique<SampleQueue>();
        OrvixSampleBufferDelegate* delegate =
            [[OrvixSampleBufferDelegate alloc] init];
        delegate.sampleQueueContext = samples.get();

        AVCaptureVideoDataOutput* output =
            [[AVCaptureVideoDataOutput alloc] init];
        output.alwaysDiscardsLateVideoFrames = NO;

        dispatch_queue_t callback_queue = dispatch_queue_create(
            "com.orvix.capture.frames",
            DISPATCH_QUEUE_SERIAL
        );
        [output setSampleBufferDelegate:delegate queue:callback_queue];

        if (![impl_->session canAddOutput:output]) {
            [output setSampleBufferDelegate:nil queue:nullptr];
            delegate.sampleQueueContext = nullptr;
            throw std::runtime_error(
                "AVFoundation rejected the video data output."
            );
        }

        [impl_->session addOutput:output];
        impl_->samples = std::move(samples);
        impl_->delegate = delegate;
        impl_->output = output;
        impl_->callback_queue = callback_queue;
        impl_->configured_format = selected;
        impl_->format_configured = true;
        return selected;
    }
}

CaptureSummary AvFoundationCamera::capture_frames(
    const std::size_t requested_frames
) {
    @autoreleasepool {
        if (!is_open()) {
            throw std::logic_error(
                "A camera must be open before capturing frames."
            );
        }

        if (!impl_->format_configured || impl_->samples == nullptr) {
            throw std::logic_error(
                "A video format must be configured before capturing frames."
            );
        }

        {
            const std::lock_guard lock(impl_->samples->mutex);
            impl_->samples->frames.clear();
            impl_->samples->callback_failed = false;
        }

        [impl_->session startRunning];

        try {
            AvFoundationFrameReader reader(
                *impl_->samples,
                impl_->device
            );
            CaptureSummary summary = ContinuousFrameCapture::run(
                reader,
                requested_frames,
                impl_->configured_format,
                20
            );
            [impl_->session stopRunning];
            return summary;
        }
        catch (...) {
            [impl_->session stopRunning];
            throw;
        }
    }
}

bool AvFoundationCamera::is_open() const noexcept {
    return
        impl_ != nullptr &&
        impl_->device != nil &&
        impl_->session != nil;
}

}  // namespace orvix::capture
