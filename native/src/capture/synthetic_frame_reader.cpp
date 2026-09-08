#include "orvix/capture/synthetic_frame_reader.hpp"

#include <algorithm>
#include <chrono>
#include <limits>
#include <stdexcept>
#include <thread>
#include <utility>
#include <vector>

namespace orvix::capture {

namespace {

std::size_t nv12_size(
    const std::uint32_t width,
    const std::uint32_t height
) {
    const std::size_t pixels =
        static_cast<std::size_t>(width) * height;
    if (pixels > std::numeric_limits<std::size_t>::max() / 3U) {
        throw std::overflow_error("Synthetic frame size overflowed.");
    }
    return pixels + pixels / 2U;
}

}  // namespace

SyntheticFrameReader::SyntheticFrameReader(
    const std::uint32_t width,
    const std::uint32_t height,
    const std::uint32_t frames_per_second,
    const bool realtime
)
    : format_{
          0,
          width,
          height,
          frames_per_second,
          1,
          "NV12",
          false,
          width
      },
      realtime_(realtime),
      next_frame_at_(std::chrono::steady_clock::now()) {
    if (
        width == 0 ||
        height == 0 ||
        width % 2U != 0 ||
        height % 2U != 0 ||
        frames_per_second == 0
    ) {
        throw std::invalid_argument(
            "Synthetic NV12 frames require non-zero even dimensions and FPS."
        );
    }
}

FrameReadResult SyntheticFrameReader::read_next() {
    const auto interval = std::chrono::nanoseconds(
        1'000'000'000LL / format_.frame_rate_numerator
    );
    if (realtime_) {
        next_frame_at_ += interval;
        std::this_thread::sleep_until(next_frame_at_);
    }

    ++generated_frames_;
    const std::size_t y_plane_size =
        static_cast<std::size_t>(format_.width) * format_.height;
    std::vector<std::byte> payload(nv12_size(format_.width, format_.height));
    const std::uint32_t phase = static_cast<std::uint32_t>(
        generated_frames_ % format_.width
    );

    for (std::uint32_t row = 0; row < format_.height; ++row) {
        for (std::uint32_t column = 0; column < format_.width; ++column) {
            const std::uint32_t value =
                32U + ((column + phase + row / 4U) % 192U);
            payload[static_cast<std::size_t>(row) * format_.width + column] =
                static_cast<std::byte>(value);
        }
    }
    std::fill(
        payload.begin() + static_cast<std::ptrdiff_t>(y_plane_size),
        payload.end(),
        static_cast<std::byte>(128U)
    );

    const std::int64_t timestamp = static_cast<std::int64_t>(
        generated_frames_ * 10'000'000ULL /
        format_.frame_rate_numerator
    );
    return {
        true,
        timestamp,
        payload.size(),
        format_.stride,
        std::move(payload)
    };
}

const VideoFormat& SyntheticFrameReader::format() const noexcept {
    return format_;
}

}  // namespace orvix::capture
