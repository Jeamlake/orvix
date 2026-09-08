#include "orvix/ipc/shared_memory_publisher.hpp"

#include "orvix/ipc/shared_memory_protocol.hpp"

#include <algorithm>
#include <array>
#include <atomic>
#include <bit>
#include <chrono>
#include <cmath>
#include <cstring>
#include <limits>
#include <memory>
#include <ratio>
#include <stdexcept>
#include <type_traits>

#if defined(_WIN32)
#include <Windows.h>
#else
#include <unistd.h>
#endif

namespace orvix::ipc {

namespace {

static_assert(std::endian::native == std::endian::little);

template<typename Value>
void write_value(
    std::byte* const data,
    const std::size_t offset,
    const Value value
) noexcept {
    static_assert(std::is_trivially_copyable_v<Value>);
    std::memcpy(data + offset, &value, sizeof(Value));
}

template<typename Value>
Value read_value(
    const std::byte* const data,
    const std::size_t offset
) noexcept {
    Value value{};
    std::memcpy(&value, data + offset, sizeof(Value));
    return value;
}

std::uint64_t& uint64_at(
    std::byte* const data,
    const std::size_t offset
) noexcept {
    return *reinterpret_cast<std::uint64_t*>(data + offset);
}

std::uint32_t& uint32_at(
    std::byte* const data,
    const std::size_t offset
) noexcept {
    return *reinterpret_cast<std::uint32_t*>(data + offset);
}

std::uint64_t atomic_load_u64(
    const std::byte* const data,
    const std::size_t offset
) noexcept {
    auto& value = *reinterpret_cast<std::uint64_t*>(
        const_cast<std::byte*>(data) + offset
    );
    return std::atomic_ref<std::uint64_t>(value).load(
        std::memory_order_acquire
    );
}

void atomic_store_u64(
    std::byte* const data,
    const std::size_t offset,
    const std::uint64_t value
) noexcept {
    std::atomic_ref<std::uint64_t>(uint64_at(data, offset)).store(
        value,
        std::memory_order_release
    );
}

void atomic_store_u32(
    std::byte* const data,
    const std::size_t offset,
    const std::uint32_t value
) noexcept {
    std::atomic_ref<std::uint32_t>(uint32_at(data, offset)).store(
        value,
        std::memory_order_release
    );
}

std::size_t align_up(
    const std::size_t value,
    const std::size_t alignment
) {
    if (value > std::numeric_limits<std::size_t>::max() - alignment + 1U) {
        throw std::overflow_error("Shared-memory alignment overflowed.");
    }
    return (value + alignment - 1U) / alignment * alignment;
}

std::size_t checked_multiply(
    const std::size_t left,
    const std::size_t right
) {
    if (
        right != 0 &&
        left > std::numeric_limits<std::size_t>::max() / right
    ) {
        throw std::overflow_error("Shared-memory size overflowed.");
    }
    return left * right;
}

std::uint64_t process_id() noexcept {
#if defined(_WIN32)
    return static_cast<std::uint64_t>(GetCurrentProcessId());
#else
    return static_cast<std::uint64_t>(::getpid());
#endif
}

std::int64_t timestamp_100ns_now() noexcept {
    return std::chrono::duration_cast<
        std::chrono::duration<std::int64_t, std::ratio<1, 10'000'000>>
    >(std::chrono::system_clock::now().time_since_epoch()).count();
}

}  // namespace

PixelFormat pixel_format_code(const std::string_view name) noexcept {
    if (name == "NV12") return PixelFormat::nv12;
    if (name == "YUY2") return PixelFormat::yuy2;
    if (name == "UYVY") return PixelFormat::uyvy;
    if (name == "I420" || name == "IYUV") return PixelFormat::i420;
    if (name == "YV12") return PixelFormat::yv12;
    if (name == "RGB24") return PixelFormat::rgb24;
    if (name == "RGB32" || name == "BGRA32") return PixelFormat::bgra32;
    if (name == "ARGB32") return PixelFormat::argb32;
    if (name == "MJPG") return PixelFormat::mjpg;
    if (name == "H264") return PixelFormat::h264;
    return PixelFormat::unknown;
}

std::string_view pixel_format_name(const PixelFormat format) noexcept {
    switch (format) {
    case PixelFormat::nv12: return "NV12";
    case PixelFormat::yuy2: return "YUY2";
    case PixelFormat::uyvy: return "UYVY";
    case PixelFormat::i420: return "I420";
    case PixelFormat::yv12: return "YV12";
    case PixelFormat::rgb24: return "RGB24";
    case PixelFormat::bgra32: return "BGRA32";
    case PixelFormat::argb32: return "ARGB32";
    case PixelFormat::mjpg: return "MJPG";
    case PixelFormat::h264: return "H264";
    default: return "UNKNOWN";
    }
}

std::size_t default_payload_capacity(const capture::VideoFormat& format) {
    if (format.width == 0 || format.height == 0) {
        throw std::invalid_argument(
            "Shared memory requires non-zero frame dimensions."
        );
    }
    const std::size_t pixels = checked_multiply(format.width, format.height);
    return checked_multiply(pixels, 4U);
}

SharedMemoryPublisher::SharedMemoryPublisher(
    std::string name,
    const capture::VideoFormat& format,
    const std::uint32_t slot_count,
    std::size_t payload_capacity
)
    : format_(format),
      slot_count_(slot_count) {
    if (slot_count < 2 || slot_count > 64) {
        throw std::invalid_argument(
            "The shared-memory ring requires between 2 and 64 slots."
        );
    }
    if (format.frame_rate_denominator == 0) {
        throw std::invalid_argument("Frame-rate denominator cannot be zero.");
    }
    if (payload_capacity == 0) {
        payload_capacity = default_payload_capacity(format);
    }
    payload_capacity_ = payload_capacity;
    slot_stride_ = align_up(kSlotHeaderSize + payload_capacity_, 64U);
    const std::size_t total_size = kGlobalHeaderSize + checked_multiply(
        slot_stride_,
        slot_count_
    );
    region_ = std::make_unique<SharedMemoryRegion>(
        std::move(name),
        total_size
    );

    std::byte* const data = region_->data();
    std::memset(data, 0, total_size);
    std::construct_at(
        reinterpret_cast<std::uint32_t*>(
            data + header_offset::producer_state
        ),
        static_cast<std::uint32_t>(ProducerState::initializing)
    );
    std::construct_at(
        reinterpret_cast<std::uint32_t*>(
            data + header_offset::published_slot
        ),
        0U
    );
    for (std::uint32_t slot = 0; slot < slot_count_; ++slot) {
        std::construct_at(
            reinterpret_cast<std::uint64_t*>(
                data + kGlobalHeaderSize +
                static_cast<std::size_t>(slot) * slot_stride_ +
                slot_offset::generation
            ),
            0ULL
        );
    }

    const std::array<char, 4> magic{'O', 'R', 'V', 'X'};
    std::memcpy(data + header_offset::magic, magic.data(), magic.size());
    write_value<std::uint16_t>(data, header_offset::version, kProtocolVersion);
    write_value<std::uint16_t>(
        data,
        header_offset::header_size,
        static_cast<std::uint16_t>(kGlobalHeaderSize)
    );
    write_value<std::uint64_t>(data, header_offset::total_size, total_size);
    write_value<std::uint32_t>(data, header_offset::slot_count, slot_count_);
    write_value<std::uint32_t>(
        data,
        header_offset::slot_header_size,
        static_cast<std::uint32_t>(kSlotHeaderSize)
    );
    write_value<std::uint64_t>(
        data,
        header_offset::payload_capacity,
        payload_capacity_
    );
    write_value<std::uint64_t>(data, header_offset::slot_stride, slot_stride_);
    write_value<std::uint32_t>(data, header_offset::width, format.width);
    write_value<std::uint32_t>(data, header_offset::height, format.height);
    write_value<std::uint32_t>(
        data,
        header_offset::frame_stride,
        format.stride
    );
    write_value<std::uint32_t>(
        data,
        header_offset::pixel_format,
        static_cast<std::uint32_t>(pixel_format_code(format.pixel_format))
    );
    write_value<std::uint32_t>(
        data,
        header_offset::fps_numerator,
        format.frame_rate_numerator
    );
    write_value<std::uint32_t>(
        data,
        header_offset::fps_denominator,
        format.frame_rate_denominator
    );
    write_value<std::int64_t>(
        data,
        header_offset::created_timestamp_100ns,
        timestamp_100ns_now()
    );
    write_value<std::uint64_t>(
        data,
        header_offset::producer_pid,
        process_id()
    );
    std::construct_at(
        reinterpret_cast<std::uint64_t*>(
            data + header_offset::published_sequence
        ),
        0ULL
    );
    std::construct_at(
        reinterpret_cast<std::uint64_t*>(
            data + header_offset::consumed_sequence
        ),
        0ULL
    );
    std::construct_at(
        reinterpret_cast<std::uint64_t*>(
            data + header_offset::overwritten_frames
        ),
        0ULL
    );
    std::construct_at(
        reinterpret_cast<std::uint64_t*>(
            data + header_offset::producer_fps_x1000
        ),
        0ULL
    );
    atomic_store_u32(
        data,
        header_offset::producer_state,
        static_cast<std::uint32_t>(ProducerState::running)
    );
}

SharedMemoryPublisher::~SharedMemoryPublisher() {
    if (!terminal_state_written_) {
        mark_failed();
    }
}

void SharedMemoryPublisher::publish(
    const capture::FrameMetadata& metadata,
    const std::span<const std::byte> payload
) {
    if (terminal_state_written_) {
        throw std::logic_error("Cannot publish after the producer stopped.");
    }
    if (metadata.sequence == 0) {
        throw std::invalid_argument("Published frame sequence must be positive.");
    }
    if (payload.size() != metadata.byte_count) {
        throw std::invalid_argument(
            "Published payload size does not match frame metadata."
        );
    }
    if (payload.size() > payload_capacity_) {
        throw std::length_error(
            "Captured frame exceeds the shared-memory slot capacity."
        );
    }

    std::byte* const data = region_->data();
    const std::uint32_t slot_index = static_cast<std::uint32_t>(
        (metadata.sequence - 1U) % slot_count_
    );
    std::byte* const slot = data + kGlobalHeaderSize +
        static_cast<std::size_t>(slot_index) * slot_stride_;
    auto generation = std::atomic_ref<std::uint64_t>(
        uint64_at(slot, slot_offset::generation)
    );
    std::uint64_t current_generation = generation.load(
        std::memory_order_relaxed
    );
    if ((current_generation & 1U) != 0U) {
        ++current_generation;
    }
    generation.store(current_generation + 1U, std::memory_order_release);
    write_value<std::uint32_t>(
        slot,
        slot_offset::state,
        static_cast<std::uint32_t>(SlotState::writing)
    );
    write_value<std::uint64_t>(
        slot,
        slot_offset::sequence,
        metadata.sequence
    );
    write_value<std::int64_t>(
        slot,
        slot_offset::timestamp_100ns,
        metadata.timestamp_100ns
    );
    write_value<std::uint64_t>(
        slot,
        slot_offset::payload_size,
        payload.size()
    );
    write_value<std::uint32_t>(slot, slot_offset::width, metadata.width);
    write_value<std::uint32_t>(slot, slot_offset::height, metadata.height);
    write_value<std::uint32_t>(
        slot,
        slot_offset::frame_stride,
        metadata.stride
    );
    write_value<std::uint32_t>(
        slot,
        slot_offset::pixel_format,
        static_cast<std::uint32_t>(pixel_format_code(metadata.pixel_format))
    );
    if (!payload.empty()) {
        std::memcpy(slot + slot_offset::payload, payload.data(), payload.size());
    }
    write_value<std::uint32_t>(
        slot,
        slot_offset::state,
        static_cast<std::uint32_t>(SlotState::ready)
    );
    generation.store(current_generation + 2U, std::memory_order_release);

    atomic_store_u32(data, header_offset::published_slot, slot_index);
    atomic_store_u64(
        data,
        header_offset::published_sequence,
        metadata.sequence
    );

    const std::uint64_t consumed = atomic_load_u64(
        data,
        header_offset::consumed_sequence
    );
    if (
        metadata.sequence > slot_count_ &&
        metadata.sequence - consumed > slot_count_
    ) {
        auto& overwritten = uint64_at(
            data,
            header_offset::overwritten_frames
        );
        std::atomic_ref<std::uint64_t>(overwritten).fetch_add(
            1U,
            std::memory_order_relaxed
        );
    }

    if (first_sequence_ == 0) {
        first_sequence_ = metadata.sequence;
        first_timestamp_100ns_ = metadata.timestamp_100ns;
    }
    else if (metadata.timestamp_100ns > first_timestamp_100ns_) {
        const double elapsed = static_cast<double>(
            metadata.timestamp_100ns - first_timestamp_100ns_
        ) / 10'000'000.0;
        const double fps = static_cast<double>(
            metadata.sequence - first_sequence_
        ) / elapsed;
        atomic_store_u64(
            data,
            header_offset::producer_fps_x1000,
            static_cast<std::uint64_t>(std::llround(fps * 1000.0))
        );
    }
}

void SharedMemoryPublisher::mark_completed() noexcept {
    if (region_ != nullptr) {
        atomic_store_u32(
            region_->data(),
            header_offset::producer_state,
            static_cast<std::uint32_t>(ProducerState::stopped)
        );
    }
    terminal_state_written_ = true;
}

void SharedMemoryPublisher::mark_failed() noexcept {
    if (region_ != nullptr) {
        atomic_store_u32(
            region_->data(),
            header_offset::producer_state,
            static_cast<std::uint32_t>(ProducerState::failed)
        );
    }
    terminal_state_written_ = true;
}

PublisherStats SharedMemoryPublisher::stats() const noexcept {
    const std::byte* const data = region_->data();
    PublisherStats result;
    result.published_sequence = atomic_load_u64(
        data,
        header_offset::published_sequence
    );
    result.consumed_sequence = atomic_load_u64(
        data,
        header_offset::consumed_sequence
    );
    result.overwritten_frames = atomic_load_u64(
        data,
        header_offset::overwritten_frames
    );
    result.producer_fps = static_cast<double>(atomic_load_u64(
        data,
        header_offset::producer_fps_x1000
    )) / 1000.0;
    const std::uint64_t pending = result.published_sequence >
        result.consumed_sequence
            ? result.published_sequence - result.consumed_sequence
            : 0;
    result.buffer_utilization = static_cast<double>(
        std::min<std::uint64_t>(pending, slot_count_)
    ) / static_cast<double>(slot_count_);
    return result;
}

std::span<const std::byte> SharedMemoryPublisher::memory() const noexcept {
    return {region_->data(), region_->size()};
}

const std::string& SharedMemoryPublisher::name() const noexcept {
    return region_->name();
}

}  // namespace orvix::ipc
