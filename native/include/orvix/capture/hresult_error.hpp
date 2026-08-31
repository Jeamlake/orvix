#pragma once

#include <stdexcept>
#include <string>

namespace orvix::capture {

class HResultError final : public std::runtime_error {
public:
    HResultError(long hresult, std::string operation);

    [[nodiscard]]
    long hresult() const noexcept;

private:
    long hresult_;
};

}  // namespace orvix::capture