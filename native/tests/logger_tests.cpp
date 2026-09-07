#include "orvix/observability/logger.hpp"

#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>

int main() {
    const auto unique_value =
        std::chrono::steady_clock::now().time_since_epoch().count();
    const std::filesystem::path log_path =
        std::filesystem::temp_directory_path() /
        ("orvix_logger_test_" + std::to_string(unique_value) + ".log");

    {
        orvix::observability::Logger logger(log_path);
        logger.info("ORV-TEST-001", "test_started", "logger test");
        logger.error(
            "ORV-TEST-500",
            "test_error",
            "quoted \"message\""
        );
    }

    std::ifstream input(log_path);
    const std::string content{
        std::istreambuf_iterator<char>(input),
        std::istreambuf_iterator<char>()
    };

    std::error_code remove_error;
    std::filesystem::remove(log_path, remove_error);

    if (
        content.find("level=INFO code=ORV-TEST-001 event=test_started") ==
            std::string::npos ||
        content.find("level=ERROR code=ORV-TEST-500 event=test_error") ==
            std::string::npos ||
        content.find("quoted \\\"message\\\"") == std::string::npos
    ) {
        std::cerr << "structured log assertion failed\n";
        return EXIT_FAILURE;
    }

    std::cout << "TC-LOG-001=PASS\n";
    return EXIT_SUCCESS;
}
