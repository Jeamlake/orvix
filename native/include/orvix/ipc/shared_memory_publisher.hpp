#pragma once

#include "orvix/capture/continuous_frame_capture.hpp"
#include "orvix/capture/video_format.hpp"
#include "orvix/ipc/shared_memory_region.hpp"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <span>
#include <string>

namespace orvix::ipc {

struct PublisherStats final {
    std::uint64_t published_sequence{};
    std::uint64_t consumed_sequence{};
    std::uint64_t overwritten_frames{};
    double producer_fps{};
    double buffer_utilization{};
};

class SharedMemoryPublisher final : public capture::FrameSink {
public:
    SharedMemoryPublisher(
        std::string name,
        const capture::VideoFormat& format,
        std::uint32_t slot_count = 3,
        std::size_t payload_capacity = 0
    );
    ~SharedMemoryPublisher() override;

    SharedMemoryPublisher(const SharedMemoryPublisher&) = delete;
    SharedMemoryPublisher& operator=(const SharedMemoryPublisher&) = delete;
    SharedMemoryPublisher(SharedMemoryPublisher&&) = delete;
    SharedMemoryPublisher& operator=(SharedMemoryPublisher&&) = delete;

    void publish(
        const capture::FrameMetadata& metadata,
        std::span<const std::byte> payload
    ) override;

    void mark_completed() noexcept;
    void mark_failed() noexcept;

    [[nodiscard]]
    PublisherStats stats() const noexcept;

    [[nodiscard]]
    std::span<const std::byte> memory() const noexcept;

    [[nodiscard]]
    const std::string& name() const noexcept;

private:
    std::unique_ptr<SharedMemoryRegion> region_;
    capture::VideoFormat format_;
    std::uint32_t slot_count_{};
    std::size_t payload_capacity_{};
    std::size_t slot_stride_{};
    std::uint64_t first_sequence_{};
    std::int64_t first_timestamp_100ns_{};
    bool terminal_state_written_{false};
};

}  // namespace orvix::ipc
