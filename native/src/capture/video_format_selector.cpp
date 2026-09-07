#include "orvix/capture/video_format_selector.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <stdexcept>
#include <string_view>
#include <tuple>

namespace orvix::capture {

namespace {

std::uint32_t absolute_difference(
    const std::uint32_t left,
    const std::uint32_t right
) noexcept {
    return left > right ? left - right : right - left;
}

int pixel_format_rank(const std::string_view pixel_format) noexcept {
    if (pixel_format == "NV12") {
        return 0;
    }

    if (pixel_format == "YUY2") {
        return 1;
    }

    if (pixel_format == "RGB32" || pixel_format == "BGRA32") {
        return 2;
    }

    if (pixel_format == "RGB24") {
        return 3;
    }

    if (pixel_format == "MJPG") {
        return 4;
    }

    return 5;
}

auto score(
    const VideoFormat& format,
    const VideoFormatTarget& target
) noexcept {
    const bool exact_resolution =
        format.width == target.width && format.height == target.height;

    const std::uint64_t resolution_distance =
        static_cast<std::uint64_t>(
            absolute_difference(format.width, target.width)
        ) +
        static_cast<std::uint64_t>(
            absolute_difference(format.height, target.height)
        );

    const double frame_rate_distance = std::abs(
        format.frames_per_second() -
        static_cast<double>(target.frames_per_second)
    );

    return std::tuple{
        !exact_resolution,
        resolution_distance,
        frame_rate_distance,
        pixel_format_rank(format.pixel_format),
        format.native_type_index
    };
}

}  // namespace

const VideoFormat& VideoFormatSelector::select_best(
    const std::vector<VideoFormat>& formats,
    const VideoFormatTarget& target
) {
    if (formats.empty()) {
        throw std::runtime_error(
            "The selected camera does not expose a usable video format."
        );
    }

    return *std::min_element(
        formats.begin(),
        formats.end(),
        [&target](const VideoFormat& left, const VideoFormat& right) {
            return score(left, target) < score(right, target);
        }
    );
}

}  // namespace orvix::capture
