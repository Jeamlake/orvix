#pragma once

#include "orvix/capture/video_format.hpp"

#include <cstddef>
#include <cstdint>
#include <string_view>

namespace orvix::ipc {

inline constexpr std::string_view kDefaultSharedMemoryName =
    "orvix-camera-v1";
inline constexpr std::uint16_t kProtocolVersion = 1;
inline constexpr std::size_t kGlobalHeaderSize = 128;
inline constexpr std::size_t kSlotHeaderSize = 64;
inline constexpr std::uint32_t kDefaultSlotCount = 3;

enum class PixelFormat : std::uint32_t {
    unknown = 0,
    nv12 = 1,
    yuy2 = 2,
    uyvy = 3,
    i420 = 4,
    yv12 = 5,
    rgb24 = 6,
    bgra32 = 7,
    argb32 = 8,
    mjpg = 9,
    h264 = 10
};

enum class ProducerState : std::uint32_t {
    initializing = 0,
    running = 1,
    stopped = 2,
    failed = 3
};

enum class SlotState : std::uint32_t {
    empty = 0,
    writing = 1,
    ready = 2
};

namespace header_offset {
inline constexpr std::size_t magic = 0;
inline constexpr std::size_t version = 4;
inline constexpr std::size_t header_size = 6;
inline constexpr std::size_t total_size = 8;
inline constexpr std::size_t slot_count = 16;
inline constexpr std::size_t slot_header_size = 20;
inline constexpr std::size_t payload_capacity = 24;
inline constexpr std::size_t slot_stride = 32;
inline constexpr std::size_t width = 40;
inline constexpr std::size_t height = 44;
inline constexpr std::size_t frame_stride = 48;
inline constexpr std::size_t pixel_format = 52;
inline constexpr std::size_t fps_numerator = 56;
inline constexpr std::size_t fps_denominator = 60;
inline constexpr std::size_t created_timestamp_100ns = 64;
inline constexpr std::size_t producer_pid = 72;
inline constexpr std::size_t producer_state = 80;
inline constexpr std::size_t published_slot = 84;
inline constexpr std::size_t published_sequence = 88;
inline constexpr std::size_t consumed_sequence = 96;
inline constexpr std::size_t overwritten_frames = 104;
inline constexpr std::size_t producer_fps_x1000 = 112;
}  // namespace header_offset

namespace slot_offset {
inline constexpr std::size_t generation = 0;
inline constexpr std::size_t sequence = 8;
inline constexpr std::size_t timestamp_100ns = 16;
inline constexpr std::size_t payload_size = 24;
inline constexpr std::size_t width = 32;
inline constexpr std::size_t height = 36;
inline constexpr std::size_t frame_stride = 40;
inline constexpr std::size_t pixel_format = 44;
inline constexpr std::size_t state = 48;
inline constexpr std::size_t payload = kSlotHeaderSize;
}  // namespace slot_offset

[[nodiscard]]
PixelFormat pixel_format_code(std::string_view name) noexcept;

[[nodiscard]]
std::string_view pixel_format_name(PixelFormat format) noexcept;

[[nodiscard]]
std::size_t default_payload_capacity(
    const capture::VideoFormat& format
);

}  // namespace orvix::ipc
