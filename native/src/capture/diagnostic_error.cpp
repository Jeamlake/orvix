#include "orvix/capture/diagnostic_error.hpp"

#include <utility>

namespace orvix::capture {

DiagnosticError::DiagnosticError(
    std::string code,
    std::string event,
    std::string message,
    const int exit_code
)
    : std::runtime_error(std::move(message)),
      code_(std::move(code)),
      event_(std::move(event)),
      exit_code_(exit_code) {}

const std::string& DiagnosticError::code() const noexcept {
    return code_;
}

const std::string& DiagnosticError::event() const noexcept {
    return event_;
}

int DiagnosticError::exit_code() const noexcept {
    return exit_code_;
}

}  // namespace orvix::capture
