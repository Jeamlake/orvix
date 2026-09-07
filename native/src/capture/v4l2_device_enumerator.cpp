#include "orvix/capture/v4l2_device_enumerator.hpp"

#include <fcntl.h>
#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <algorithm>
#include <cctype>
#include <cerrno>
#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace orvix::capture {

namespace {

bool is_video_device_name(const std::string& name) {
    constexpr auto prefix = "video";

    if (name.rfind(prefix, 0) != 0 || name.size() == 5) {
        return false;
    }

    return std::all_of(
        name.begin() + 5,
        name.end(),
        [](const unsigned char value) {
            return std::isdigit(value) != 0;
        }
    );
}

std::string capability_name(const v4l2_capability& capability) {
    const auto* begin = reinterpret_cast<const char*>(capability.card);
    const auto* end = std::find(begin, begin + sizeof(capability.card), '\0');
    return std::string(begin, end);
}

bool supports_video_capture(const v4l2_capability& capability) noexcept {
    const std::uint32_t capabilities =
        (capability.capabilities & V4L2_CAP_DEVICE_CAPS) != 0
            ? capability.device_caps
            : capability.capabilities;

    return
        (capabilities & V4L2_CAP_VIDEO_CAPTURE) != 0 &&
        (capabilities & V4L2_CAP_STREAMING) != 0;
}

}  // namespace

std::vector<CameraDevice> V4l2DeviceEnumerator::enumerate() const {
    std::vector<std::filesystem::path> paths;
    std::error_code error;

    for (
        const auto& entry :
            std::filesystem::directory_iterator("/dev", error)
    ) {
        if (error) {
            break;
        }

        const std::string name = entry.path().filename().string();
        if (is_video_device_name(name)) {
            paths.push_back(entry.path());
        }
    }

    std::sort(paths.begin(), paths.end());

    std::vector<CameraDevice> devices;

    for (const auto& path : paths) {
        const std::string native_path = path.string();
        const int descriptor = ::open(
            native_path.c_str(),
            O_RDWR | O_NONBLOCK
        );

        if (descriptor < 0) {
            if (errno == EACCES || errno == EBUSY) {
                devices.push_back({
                    devices.size(),
                    path.filename().string(),
                    native_path,
                    "Linux V4L2"
                });
            }
            continue;
        }

        v4l2_capability capability{};
        const bool usable =
            ::ioctl(descriptor, VIDIOC_QUERYCAP, &capability) == 0 &&
            supports_video_capture(capability);
        static_cast<void>(::close(descriptor));

        if (!usable) {
            continue;
        }

        std::string friendly_name = capability_name(capability);
        if (friendly_name.empty()) {
            friendly_name = path.filename().string();
        }

        devices.push_back({
            devices.size(),
            std::move(friendly_name),
            native_path,
            "Linux V4L2"
        });
    }

    return devices;
}

}  // namespace orvix::capture
