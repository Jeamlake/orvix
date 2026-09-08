#pragma once

#include "orvix/capture/video_format.hpp"

#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <vector>

namespace orvix::capture {

struct FrameReadResult final {
    bool frame_available{false};
    std::int64_t timestamp_100ns{};
    std::size_t byte_count{};
    std::uint32_t stride{};
    std::vector<std::byte> payload;
};

class FrameReader {
public:
    virtual ~FrameReader() = default;

    [[nodiscard]]
    virtual FrameReadResult read_next() = 0;
};

struct FrameMetadata final {
    std::uint64_t sequence{};
    std::int64_t timestamp_100ns{};
    std::uint32_t width{};
    std::uint32_t height{};
    std::uint32_t stride{};
    std::string pixel_format;
    std::size_t byte_count{};
};

class FrameSink {
public:
    virtual ~FrameSink() = default;

    virtual void publish(
        const FrameMetadata& metadata,
        std::span<const std::byte> payload
    ) = 0;
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
    bool timestamps_strictly_increasing{true};
    std::vector<FrameMetadata> frames;

    [[nodiscard]]
    double duration_seconds() const noexcept;

    [[nodiscard]]
    double capture_fps() const noexcept;
};

class ContinuousFrameCapture final {
public:
    [[nodiscard]]
    static CaptureSummary run(
        FrameReader& reader,
        std::size_t requested_frames,
        const VideoFormat& format,
        std::size_t maximum_empty_reads = 1000,
        FrameSink* sink = nullptr
    );
};

}  // namespace orvix::capture
