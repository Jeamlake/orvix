#pragma once

#include "orvix/capture/video_format.hpp"

#include <vector>

namespace orvix::capture {

class VideoFormatSelector final {
public:
    [[nodiscard]]
    static const VideoFormat& select_best(
        const std::vector<VideoFormat>& formats,
        const VideoFormatTarget& target
    );
};

}  // namespace orvix::capture
