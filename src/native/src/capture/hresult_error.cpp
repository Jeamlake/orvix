#include "orvix/capture/hresult_error.hpp"

#include <iomanip>
#include <sstream>
#include <utility>

namespace orvix::capture {

namespace {

std::string build_message(const long hresult, const std::string& operation) {
    std::ostringstream stream;

    stream << operation
           << " failed with HRESULT 0x"
           << std::uppercase
           << std::hex
           << static_cast<unsigned long>(hresult);

    return stream.str();
}

}  // namespace

HResultError::HResultError(const long hresult, std::string operation)
    : std::runtime_error(build_message(hresult, operation)),
      hresult_(hresult) {}

long HResultError::hresult() const noexcept {
    return hresult_;
}

}  // namespace orvix::capture