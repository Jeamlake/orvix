#pragma once

namespace orvix::capture {

class ComRuntime final {
public:
    ComRuntime();
    ~ComRuntime();

    ComRuntime(const ComRuntime&) = delete;
    ComRuntime& operator=(const ComRuntime&) = delete;
    ComRuntime(ComRuntime&&) = delete;
    ComRuntime& operator=(ComRuntime&&) = delete;

private:
    bool initialized_{false};
};

class MediaFoundationRuntime final {
public:
    MediaFoundationRuntime();
    ~MediaFoundationRuntime();

    MediaFoundationRuntime(const MediaFoundationRuntime&) = delete;
    MediaFoundationRuntime& operator=(const MediaFoundationRuntime&) = delete;
    MediaFoundationRuntime(MediaFoundationRuntime&&) = delete;
    MediaFoundationRuntime& operator=(MediaFoundationRuntime&&) = delete;

private:
    bool initialized_{false};
};

}  // namespace orvix::capture
