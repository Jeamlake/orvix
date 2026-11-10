#include "orvix/capture/video_format_selector.hpp"

#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <vector>

int main() {
    using orvix::capture::VideoFormat;
    using orvix::capture::VideoFormatSelector;
    using orvix::capture::VideoFormatTarget;

    const std::vector<VideoFormat> formats{
        {0, 640, 480, 30, 1, "YUY2", false},
        {1, 1280, 720, 15, 1, "YUY2", false},
        {2, 1280, 720, 30, 1, "MJPG", true},
        {3, 1280, 720, 30, 1, "NV12", false},
        {4, 1920, 1080, 30, 1, "MJPG", true}
    };

    const auto& selected = VideoFormatSelector::select_best(
        formats,
        VideoFormatTarget{}
    );

    if (selected.native_type_index != 3) {
        std::cerr << "exact target or pixel-format preference failed\n";
        return EXIT_FAILURE;
    }

    const std::vector<VideoFormat> fallback_formats{
        {0, 640, 480, 30, 1, "YUY2", false},
        {1, 1920, 1080, 60, 1, "MJPG", true}
    };

    const auto& fallback = VideoFormatSelector::select_best(
        fallback_formats,
        VideoFormatTarget{}
    );

    if (fallback.native_type_index != 0) {
        std::cerr << "nearest-resolution fallback failed\n";
        return EXIT_FAILURE;
    }

    bool empty_list_rejected = false;

    try {
        const std::vector<VideoFormat> empty;
        static_cast<void>(
            VideoFormatSelector::select_best(empty, VideoFormatTarget{})
        );
    }
    catch (const std::runtime_error&) {
        empty_list_rejected = true;
    }

    if (!empty_list_rejected) {
        std::cerr << "empty format list was not rejected\n";
        return EXIT_FAILURE;
    }

    std::cout << "TC-CAP-FORMAT=PASS\n";
    return EXIT_SUCCESS;
}
