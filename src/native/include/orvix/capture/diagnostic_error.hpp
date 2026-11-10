#pragma once

#include <stdexcept>
#include <string>

namespace orvix::capture {

class DiagnosticError final : public std::runtime_error {
public:
    DiagnosticError(
        std::string code,
        std::string event,
        std::string message,
        int exit_code
    );

    [[nodiscard]]
    const std::string& code() const noexcept;

    [[nodiscard]]
    const std::string& event() const noexcept;

    [[nodiscard]]
    int exit_code() const noexcept;

private:
    std::string code_;
    std::string event_;
    int exit_code_;
};

}  // namespace orvix::capture
