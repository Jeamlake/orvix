#include "orvix/capture/continuous_frame_capture.hpp"
#include "orvix/capture/diagnostic_error.hpp"

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <utility>
#include <vector>

namespace {

class SyntheticFrameReader final : public orvix::capture::FrameReader {
public:
    explicit SyntheticFrameReader(
        std::vector<orvix::capture::FrameReadResult> reads
    )
        : reads_(std::move(reads)) {}

    orvix::capture::FrameReadResult read_next() override {
        if (position_ >= reads_.size()) {
            return {};
        }

        return reads_[position_++];
    }

private:
    std::vector<orvix::capture::FrameReadResult> reads_;
    std::size_t position_{};
};

}  // namespace

int main() {
    SyntheticFrameReader reader({
        {false, 0, 0},
        {true, 1000000, 10},
        {true, 1333333, 20},
        {true, 1666666, 30}
    });

    const auto summary =
        orvix::capture::ContinuousFrameCapture::run(
            reader,
            3,
            {0, 1280, 720, 30, 1, "NV12", false}
        );

    if (
        summary.captured_frames != 3 ||
        summary.first_sequence != 1 ||
        summary.last_sequence != 3 ||
        summary.first_timestamp_100ns != 1000000 ||
        summary.last_timestamp_100ns != 1666666 ||
        summary.total_bytes != 60 ||
        summary.empty_reads != 1 ||
        !summary.timestamps_strictly_increasing ||
        summary.frames.size() != 3 ||
        summary.frames[1].sequence != 2 ||
        summary.frames[1].timestamp_100ns != 1333333 ||
        summary.frames[1].width != 1280 ||
        summary.frames[1].height != 720 ||
        summary.frames[1].pixel_format != "NV12" ||
        summary.frames[1].byte_count != 20
    ) {
        std::cerr << "continuous frame summary assertion failed\n";
        return EXIT_FAILURE;
    }

    if (std::abs(summary.capture_fps() - 30.0) > 0.01) {
        std::cerr << "capture FPS calculation assertion failed\n";
        return EXIT_FAILURE;
    }

    bool zero_frames_rejected = false;

    try {
        SyntheticFrameReader unused({});
        static_cast<void>(
            orvix::capture::ContinuousFrameCapture::run(
                unused,
                0,
                {0, 1280, 720, 30, 1, "NV12", false}
            )
        );
    }
    catch (const std::invalid_argument&) {
        zero_frames_rejected = true;
    }

    if (!zero_frames_rejected) {
        std::cerr << "zero-frame request was not rejected\n";
        return EXIT_FAILURE;
    }

    bool stalled_stream_rejected = false;

    try {
        SyntheticFrameReader stalled({});
        static_cast<void>(
            orvix::capture::ContinuousFrameCapture::run(
                stalled,
                1,
                {0, 1280, 720, 30, 1, "NV12", false},
                2
            )
        );
    }
    catch (const orvix::capture::DiagnosticError& error) {
        stalled_stream_rejected =
            error.code() == "ORV-CAP-503" &&
            error.event() == "camera_stream_stalled" &&
            error.exit_code() == 74;
    }

    if (!stalled_stream_rejected) {
        std::cerr << "stalled camera stream was not diagnosed\n";
        return EXIT_FAILURE;
    }

    std::cout << "TC-CAP-004=PASS\n";
    return EXIT_SUCCESS;
}
