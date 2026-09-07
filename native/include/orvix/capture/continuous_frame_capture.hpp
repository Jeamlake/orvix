#pragma once

#include <cstddef>
#include <cstdint>

namespace orvix::capture {

struct FrameReadResult final {
    bool frame_available{false};
    std::int64_t timestamp_100ns{};
    std::size_t byte_count{};
};

class FrameReader {
public:
    virtual ~FrameReader() = default;

    [[nodiscard]]
    virtual FrameReadResult read_next() = 0;
};

struct CaptureSummary final {
    std::size_t requested_frames{};
    std::size_t captured_frames{};
    std::uint64_t first_sequence{};
    std::uint64_t last_sequence{};
    std::int64_t first_timestamp_100ns{};
    std::int64_t last_timestamp_100ns{};
    std::size_t total_bytes{};
    std::size_t empty_reads{};
};

class ContinuousFrameCapture final {
public:
    [[nodiscard]]
    static CaptureSummary run(
        FrameReader& reader,
        std::size_t requested_frames,
        std::size_t maximum_empty_reads = 1000
    );
};

}  // namespace orvix::capture
