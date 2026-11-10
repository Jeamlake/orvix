#pragma once

namespace orvix::cli {

class CaptureCli final {
public:
    [[nodiscard]]
    int run(int argc, char* argv[]) const;
};

}  // namespace orvix::cli
