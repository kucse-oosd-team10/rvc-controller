#pragma once

#include <cstdint>
#include <functional>

namespace rvc {
class Timer {
public:
    using Callback = std::function<void()>;
    using ClockFn = std::function<std::int64_t()>;

    explicit Timer(ClockFn clockFn = defaultClock);

    void start();
    void stop();
    void reset();

    [[nodiscard]] bool isExpired() const;
    [[nodiscard]] bool isRunning() const;

    void setDuration(std::int64_t durationMs);
    void setCallback(Callback callback);

    void update();

private:
    static std::int64_t defaultClock();

    ClockFn clock_;
    Callback callback_;

    std::int64_t duration_{0};
    std::int64_t startTime_{0};

    bool isRunning_{false};
    bool isFired_{false};
};
} // namespace rvc
