#pragma once

#include "orvix/capture/continuous_frame_capture.hpp"
#include "orvix/capture/video_format.hpp"

#include <chrono>
#include <cstdint>

namespace orvix::capture {

class SyntheticFrameReader final : public FrameReader {
public:
    SyntheticFrameReader(
        std::uint32_t width = 1280,
        std::uint32_t height = 720,
        std::uint32_t frames_per_second = 30,
        bool realtime = true
    );

    [[nodiscard]]
    FrameReadResult read_next() override;

    [[nodiscard]]
    const VideoFormat& format() const noexcept;

private:
    VideoFormat format_;
    std::uint64_t generated_frames_{};
    bool realtime_{true};
    std::chrono::steady_clock::time_point next_frame_at_;
};

}  // namespace orvix::capture
