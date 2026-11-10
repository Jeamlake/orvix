#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

namespace orvix::capture {

struct VideoFormat final {
    std::size_t native_type_index{};
    std::uint32_t width{};
    std::uint32_t height{};
    std::uint32_t frame_rate_numerator{};
    std::uint32_t frame_rate_denominator{1};
    std::string pixel_format;
    bool compressed{false};
    std::uint32_t stride{};

    [[nodiscard]]
    double frames_per_second() const noexcept {
        if (frame_rate_denominator == 0) {
            return 0.0;
        }

        return static_cast<double>(frame_rate_numerator) /
               static_cast<double>(frame_rate_denominator);
    }
};

struct VideoFormatTarget final {
    std::uint32_t width{1280};
    std::uint32_t height{720};
    std::uint32_t frames_per_second{30};
};

}  // namespace orvix::capture
