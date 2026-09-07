#include "orvix/capture/continuous_frame_capture.hpp"

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
        {true, 1000, 10},
        {true, 2000, 20},
        {true, 3000, 30}
    });

    const auto summary =
        orvix::capture::ContinuousFrameCapture::run(reader, 3);

    if (
        summary.captured_frames != 3 ||
        summary.first_sequence != 1 ||
        summary.last_sequence != 3 ||
        summary.first_timestamp_100ns != 1000 ||
        summary.last_timestamp_100ns != 3000 ||
        summary.total_bytes != 60 ||
        summary.empty_reads != 1
    ) {
        std::cerr << "continuous frame summary assertion failed\n";
        return EXIT_FAILURE;
    }

    bool zero_frames_rejected = false;

    try {
        SyntheticFrameReader unused({});
        static_cast<void>(
            orvix::capture::ContinuousFrameCapture::run(unused, 0)
        );
    }
    catch (const std::invalid_argument&) {
        zero_frames_rejected = true;
    }

    if (!zero_frames_rejected) {
        std::cerr << "zero-frame request was not rejected\n";
        return EXIT_FAILURE;
    }

    std::cout << "TC-CAP-004=PASS\n";
    return EXIT_SUCCESS;
}
