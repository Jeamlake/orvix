#include "orvix/capture/continuous_frame_capture.hpp"

#include <limits>
#include <stdexcept>

namespace orvix::capture {

CaptureSummary ContinuousFrameCapture::run(
    FrameReader& reader,
    const std::size_t requested_frames,
    const std::size_t maximum_empty_reads
) {
    if (requested_frames == 0) {
        throw std::invalid_argument(
            "Continuous capture requires at least one frame."
        );
    }

    CaptureSummary summary;
    summary.requested_frames = requested_frames;

    while (summary.captured_frames < requested_frames) {
        const FrameReadResult frame = reader.read_next();

        if (!frame.frame_available) {
            ++summary.empty_reads;

            if (summary.empty_reads > maximum_empty_reads) {
                throw std::runtime_error(
                    "The camera did not produce frames within the allowed reads."
                );
            }

            continue;
        }

        if (
            frame.byte_count >
            std::numeric_limits<std::size_t>::max() - summary.total_bytes
        ) {
            throw std::overflow_error("Captured byte counter overflowed.");
        }

        ++summary.captured_frames;
        summary.last_sequence =
            static_cast<std::uint64_t>(summary.captured_frames);
        summary.total_bytes += frame.byte_count;

        if (summary.captured_frames == 1) {
            summary.first_sequence = summary.last_sequence;
            summary.first_timestamp_100ns = frame.timestamp_100ns;
        }

        summary.last_timestamp_100ns = frame.timestamp_100ns;
    }

    return summary;
}

}  // namespace orvix::capture
