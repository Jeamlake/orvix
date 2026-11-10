#pragma once

#include <cstddef>
#include <string>

namespace orvix::capture {

struct CameraDevice final {
    std::size_t index{};
    std::string friendly_name;
    std::string symbolic_link;
    std::string backend;
};

}  // namespace orvix::capture