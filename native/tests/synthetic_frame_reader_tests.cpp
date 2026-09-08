#include "orvix/capture/synthetic_frame_reader.hpp"

#include <cstdlib>
#include <iostream>

int main() {
    orvix::capture::SyntheticFrameReader reader(4, 2, 30, false);
    const auto first = reader.read_next();
    const auto second = reader.read_next();

    if (
        !first.frame_available ||
        first.payload.size() != 12 ||
        first.byte_count != first.payload.size() ||
        first.stride != 4 ||
        second.timestamp_100ns <= first.timestamp_100ns ||
        first.payload == second.payload ||
        reader.format().pixel_format != "NV12"
    ) {
        std::cerr << "synthetic frame source assertion failed\n";
        return EXIT_FAILURE;
    }

    std::cout << "TC-SYN-001=PASS\n";
    return EXIT_SUCCESS;
}
