#pragma once

#include <cstddef>
#include <memory>
#include <string>

namespace orvix::ipc {

class SharedMemoryRegion final {
public:
    SharedMemoryRegion(std::string name, std::size_t size);
    ~SharedMemoryRegion();

    SharedMemoryRegion(const SharedMemoryRegion&) = delete;
    SharedMemoryRegion& operator=(const SharedMemoryRegion&) = delete;
    SharedMemoryRegion(SharedMemoryRegion&&) = delete;
    SharedMemoryRegion& operator=(SharedMemoryRegion&&) = delete;

    [[nodiscard]]
    std::byte* data() noexcept;

    [[nodiscard]]
    const std::byte* data() const noexcept;

    [[nodiscard]]
    std::size_t size() const noexcept;

    [[nodiscard]]
    const std::string& name() const noexcept;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

}  // namespace orvix::ipc
