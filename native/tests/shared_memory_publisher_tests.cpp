#include "orvix/ipc/shared_memory_protocol.hpp"
#include "orvix/ipc/shared_memory_publisher.hpp"

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <span>
#include <string>
#include <vector>

namespace {

template<typename Value>
Value read_value(
    const std::span<const std::byte> memory,
    const std::size_t offset
) {
    Value value{};
    std::memcpy(&value, memory.data() + offset, sizeof(Value));
    return value;
}

std::string unique_name() {
    const auto value = std::chrono::steady_clock::now()
        .time_since_epoch().count();
    return "orvix-test-" + std::to_string(value % 1'000'000'000LL);
}

}  // namespace

int main() {
    using namespace orvix;
    using namespace orvix::ipc;

    const capture::VideoFormat format{
        0,
        4,
        2,
        30,
        1,
        "NV12",
        false,
        4
    };
    SharedMemoryPublisher publisher(unique_name(), format, 3, 12);
    std::vector<std::byte> payload(12, static_cast<std::byte>(42));

    for (std::uint64_t sequence = 1; sequence <= 5; ++sequence) {
        const capture::FrameMetadata metadata{
            sequence,
            static_cast<std::int64_t>(sequence * 333'333),
            format.width,
            format.height,
            format.stride,
            format.pixel_format,
            payload.size()
        };
        publisher.publish(metadata, payload);
    }

    const auto memory = publisher.memory();
    const std::uint32_t published_slot = read_value<std::uint32_t>(
        memory,
        header_offset::published_slot
    );
    const std::size_t slot_base = kGlobalHeaderSize +
        static_cast<std::size_t>(published_slot) *
        read_value<std::uint64_t>(memory, header_offset::slot_stride);
    const auto statistics = publisher.stats();

    if (
        std::memcmp(memory.data(), "ORVX", 4) != 0 ||
        read_value<std::uint16_t>(memory, header_offset::version) != 1 ||
        read_value<std::uint32_t>(memory, header_offset::slot_count) != 3 ||
        read_value<std::uint64_t>(
            memory,
            header_offset::published_sequence
        ) != 5 ||
        read_value<std::uint64_t>(
            memory,
            slot_base + slot_offset::sequence
        ) != 5 ||
        read_value<std::uint64_t>(
            memory,
            slot_base + slot_offset::payload_size
        ) != payload.size() ||
        read_value<std::uint32_t>(
            memory,
            slot_base + slot_offset::pixel_format
        ) != static_cast<std::uint32_t>(PixelFormat::nv12) ||
        std::memcmp(
            memory.data() + slot_base + slot_offset::payload,
            payload.data(),
            payload.size()
        ) != 0 ||
        statistics.published_sequence != 5 ||
        statistics.overwritten_frames != 2 ||
        statistics.buffer_utilization != 1.0
    ) {
        std::cerr << "shared-memory publication assertion failed\n";
        return EXIT_FAILURE;
    }

    publisher.mark_completed();
    if (
        read_value<std::uint32_t>(
            memory,
            header_offset::producer_state
        ) != static_cast<std::uint32_t>(ProducerState::stopped)
    ) {
        std::cerr << "producer state assertion failed\n";
        return EXIT_FAILURE;
    }

    std::cout << "TC-IPC-001=PASS\n";
    return EXIT_SUCCESS;
}
