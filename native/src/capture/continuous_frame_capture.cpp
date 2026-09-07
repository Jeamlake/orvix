#include "orvix/capture/continuous_frame_capture.hpp"

#include "orvix/capture/diagnostic_error.hpp"

#include <limits>
#include <stdexcept>

namespace orvix::capture {

double CaptureSummary::duration_seconds() const noexcept {
    if (
        captured_frames < 2 ||
        last_timestamp_100ns <= first_timestamp_100ns
    ) {
        return 0.0;
    }

    constexpr double kTimestampUnitsPerSecond = 10'000'000.0;
    return static_cast<double>(
        last_timestamp_100ns - first_timestamp_100ns
    ) / kTimestampUnitsPerSecond;
}

double CaptureSummary::capture_fps() const noexcept {
    const double duration = duration_seconds();

    if (captured_frames < 2 || duration <= 0.0) {
        return 0.0;
    }

    return static_cast<double>(captured_frames - 1) / duration;
}

CaptureSummary ContinuousFrameCapture::run(
    FrameReader& reader,
    const std::size_t requested_frames,
    const VideoFormat& format,
    const std::size_t maximum_empty_reads
) {
    if (requested_frames == 0) {
        throw std::invalid_argument(
            "Continuous capture requires at least one frame."
        );
    }

    CaptureSummary summary;
    summary.requested_frames = requested_frames;
    summary.frames.reserve(requested_frames);

    while (summary.captured_frames < requested_frames) {
        const FrameReadResult frame = reader.read_next();

        if (!frame.frame_available) {
            ++summary.empty_reads;

            if (summary.empty_reads > maximum_empty_reads) {
                throw DiagnosticError(
                    "ORV-CAP-503",
                    "camera_stream_stalled",
                    "The camera stopped producing video frames.",
                    74
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
        else if (frame.timestamp_100ns <= summary.last_timestamp_100ns) {
            summary.timestamps_strictly_increasing = false;
        }

        summary.last_timestamp_100ns = frame.timestamp_100ns;
        summary.frames.push_back({
            summary.last_sequence,
            frame.timestamp_100ns,
            format.width,
            format.height,
            format.pixel_format,
            frame.byte_count
        });
    }

    return summary;
}

}  // namespace orvix::capture
