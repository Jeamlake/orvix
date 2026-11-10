#pragma once

#include <filesystem>
#include <fstream>
#include <mutex>
#include <string_view>

namespace orvix::observability {

enum class LogLevel {
    info,
    warning,
    error
};

class Logger final {
public:
    explicit Logger(
        std::filesystem::path path = default_log_path()
    );

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    Logger(Logger&&) = delete;
    Logger& operator=(Logger&&) = delete;

    void write(
        LogLevel level,
        std::string_view code,
        std::string_view event,
        std::string_view message
    );

    void info(
        std::string_view code,
        std::string_view event,
        std::string_view message
    );

    void warning(
        std::string_view code,
        std::string_view event,
        std::string_view message
    );

    void error(
        std::string_view code,
        std::string_view event,
        std::string_view message
    );

    [[nodiscard]]
    const std::filesystem::path& path() const noexcept;

    [[nodiscard]]
    static std::filesystem::path default_log_path();

private:
    std::filesystem::path path_;
    std::ofstream stream_;
    std::mutex mutex_;
};

}  // namespace orvix::observability
