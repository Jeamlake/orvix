#include "orvix/observability/logger.hpp"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <string>
#include <system_error>
#include <utility>

namespace orvix::observability {

namespace {

std::string_view level_name(const LogLevel level) noexcept {
    switch (level) {
    case LogLevel::info:
        return "INFO";
    case LogLevel::warning:
        return "WARNING";
    case LogLevel::error:
        return "ERROR";
    }

    return "UNKNOWN";
}

std::string utc_timestamp() {
    const auto now = std::chrono::system_clock::now();
    const std::time_t value = std::chrono::system_clock::to_time_t(now);
    std::tm utc{};

#if defined(_WIN32)
    const bool conversion_failed = gmtime_s(&utc, &value) != 0;
#else
    const bool conversion_failed = gmtime_r(&value, &utc) == nullptr;
#endif

    if (conversion_failed) {
        throw std::runtime_error("Unable to create a UTC log timestamp.");
    }

    std::ostringstream stream;
    stream << std::put_time(&utc, "%Y-%m-%dT%H:%M:%SZ");
    return stream.str();
}

std::string escape_message(const std::string_view message) {
    std::string escaped;
    escaped.reserve(message.size());

    for (const char value : message) {
        switch (value) {
        case '\\':
            escaped += "\\\\";
            break;
        case '"':
            escaped += "\\\"";
            break;
        case '\r':
        case '\n':
            escaped += ' ';
            break;
        default:
            escaped += value;
            break;
        }
    }

    return escaped;
}

}  // namespace

Logger::Logger(std::filesystem::path path)
    : path_(std::move(path)) {
    const std::filesystem::path parent = path_.parent_path();

    if (!parent.empty()) {
        std::error_code error;
        std::filesystem::create_directories(parent, error);

        if (error) {
            throw std::runtime_error(
                "Unable to create log directory: " + error.message()
            );
        }
    }

    stream_.open(path_, std::ios::app);

    if (!stream_.is_open()) {
        throw std::runtime_error(
            "Unable to open capture log: " + path_.string()
        );
    }
}

void Logger::write(
    const LogLevel level,
    const std::string_view code,
    const std::string_view event,
    const std::string_view message
) {
    const std::lock_guard lock(mutex_);

    stream_
        << "timestamp=" << utc_timestamp()
        << " level=" << level_name(level)
        << " code=" << code
        << " event=" << event
        << " message=\"" << escape_message(message) << "\"\n";
    stream_.flush();

    if (!stream_) {
        throw std::runtime_error(
            "Unable to write capture log: " + path_.string()
        );
    }
}

void Logger::info(
    const std::string_view code,
    const std::string_view event,
    const std::string_view message
) {
    write(LogLevel::info, code, event, message);
}

void Logger::warning(
    const std::string_view code,
    const std::string_view event,
    const std::string_view message
) {
    write(LogLevel::warning, code, event, message);
}

void Logger::error(
    const std::string_view code,
    const std::string_view event,
    const std::string_view message
) {
    write(LogLevel::error, code, event, message);
}

const std::filesystem::path& Logger::path() const noexcept {
    return path_;
}

std::filesystem::path Logger::default_log_path() {
    return std::filesystem::path{"logs"} / "orvix-capture.log";
}

}  // namespace orvix::observability
