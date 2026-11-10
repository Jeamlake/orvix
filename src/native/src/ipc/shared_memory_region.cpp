#include "orvix/ipc/shared_memory_region.hpp"

#include <algorithm>
#include <cctype>
#include <cerrno>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <system_error>
#include <utility>

#if defined(_WIN32)
#include <Windows.h>
#else
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

namespace orvix::ipc {

namespace {

void validate_name(const std::string& name) {
    if (name.empty() || name.size() > 30) {
        throw std::invalid_argument(
            "Shared-memory names must contain between 1 and 30 characters."
        );
    }
    const bool valid = std::all_of(
        name.begin(),
        name.end(),
        [](const unsigned char character) {
            return
                std::isalnum(character) != 0 ||
                character == '-' ||
                character == '_';
        }
    );
    if (!valid) {
        throw std::invalid_argument(
            "Shared-memory names may contain letters, digits, '-' and '_'."
        );
    }
}

#if defined(_WIN32)

std::wstring utf8_to_wide(const std::string& value) {
    const int required = MultiByteToWideChar(
        CP_UTF8,
        MB_ERR_INVALID_CHARS,
        value.c_str(),
        -1,
        nullptr,
        0
    );
    if (required <= 0) {
        throw std::system_error(
            static_cast<int>(GetLastError()),
            std::system_category(),
            "MultiByteToWideChar(shared-memory name)"
        );
    }
    std::wstring result(static_cast<std::size_t>(required), L'\0');
    if (MultiByteToWideChar(
        CP_UTF8,
        MB_ERR_INVALID_CHARS,
        value.c_str(),
        -1,
        result.data(),
        required
    ) <= 0) {
        throw std::system_error(
            static_cast<int>(GetLastError()),
            std::system_category(),
            "MultiByteToWideChar(shared-memory name)"
        );
    }
    return result;
}

#endif

}  // namespace

struct SharedMemoryRegion::Impl final {
    std::string name;
    std::size_t size{};
    std::byte* data{nullptr};
#if defined(_WIN32)
    HANDLE mapping{nullptr};
#else
    int descriptor{-1};
    std::string native_name;
#endif
};

SharedMemoryRegion::SharedMemoryRegion(
    std::string name,
    const std::size_t size
)
    : impl_(std::make_unique<Impl>()) {
    validate_name(name);
    if (size == 0) {
        throw std::invalid_argument("Shared-memory size must be positive.");
    }

    impl_->name = std::move(name);
    impl_->size = size;

#if defined(_WIN32)
    const std::uint64_t wide_size = static_cast<std::uint64_t>(size);
    const DWORD high = static_cast<DWORD>(wide_size >> 32U);
    const DWORD low = static_cast<DWORD>(wide_size & 0xffffffffULL);
    const std::wstring native_name = utf8_to_wide(impl_->name);

    impl_->mapping = CreateFileMappingW(
        INVALID_HANDLE_VALUE,
        nullptr,
        PAGE_READWRITE,
        high,
        low,
        native_name.c_str()
    );
    if (impl_->mapping == nullptr) {
        throw std::system_error(
            static_cast<int>(GetLastError()),
            std::system_category(),
            "CreateFileMappingW"
        );
    }
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        CloseHandle(impl_->mapping);
        impl_->mapping = nullptr;
        throw std::runtime_error(
            "A producer already owns shared memory '" + impl_->name + "'."
        );
    }

    void* mapped = MapViewOfFile(
        impl_->mapping,
        FILE_MAP_ALL_ACCESS,
        0,
        0,
        size
    );
    if (mapped == nullptr) {
        const DWORD error = GetLastError();
        CloseHandle(impl_->mapping);
        impl_->mapping = nullptr;
        throw std::system_error(
            static_cast<int>(error),
            std::system_category(),
            "MapViewOfFile"
        );
    }
    impl_->data = static_cast<std::byte*>(mapped);
#else
    if (size > static_cast<std::size_t>(
        std::numeric_limits<off_t>::max()
    )) {
        throw std::overflow_error("Shared-memory size exceeds off_t.");
    }

    impl_->native_name = "/" + impl_->name;
    impl_->descriptor = ::shm_open(
        impl_->native_name.c_str(),
        O_CREAT | O_EXCL | O_RDWR,
        0600
    );
    if (impl_->descriptor < 0) {
        throw std::system_error(
            errno,
            std::generic_category(),
            "shm_open(" + impl_->native_name + ")"
        );
    }
    if (::ftruncate(impl_->descriptor, static_cast<off_t>(size)) != 0) {
        const int error = errno;
        ::close(impl_->descriptor);
        impl_->descriptor = -1;
        ::shm_unlink(impl_->native_name.c_str());
        throw std::system_error(
            error,
            std::generic_category(),
            "ftruncate(shared memory)"
        );
    }

    void* mapped = ::mmap(
        nullptr,
        size,
        PROT_READ | PROT_WRITE,
        MAP_SHARED,
        impl_->descriptor,
        0
    );
    if (mapped == MAP_FAILED) {
        const int error = errno;
        ::close(impl_->descriptor);
        impl_->descriptor = -1;
        ::shm_unlink(impl_->native_name.c_str());
        throw std::system_error(
            error,
            std::generic_category(),
            "mmap(shared memory)"
        );
    }
    impl_->data = static_cast<std::byte*>(mapped);
#endif
}

SharedMemoryRegion::~SharedMemoryRegion() {
    if (impl_ == nullptr) {
        return;
    }
#if defined(_WIN32)
    if (impl_->data != nullptr) {
        UnmapViewOfFile(impl_->data);
    }
    if (impl_->mapping != nullptr) {
        CloseHandle(impl_->mapping);
    }
#else
    if (impl_->data != nullptr) {
        ::munmap(impl_->data, impl_->size);
    }
    if (impl_->descriptor >= 0) {
        ::close(impl_->descriptor);
    }
    if (!impl_->native_name.empty()) {
        ::shm_unlink(impl_->native_name.c_str());
    }
#endif
}

std::byte* SharedMemoryRegion::data() noexcept {
    return impl_->data;
}

const std::byte* SharedMemoryRegion::data() const noexcept {
    return impl_->data;
}

std::size_t SharedMemoryRegion::size() const noexcept {
    return impl_->size;
}

const std::string& SharedMemoryRegion::name() const noexcept {
    return impl_->name;
}

}  // namespace orvix::ipc
