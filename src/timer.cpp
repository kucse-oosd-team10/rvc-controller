#include "rvc/timer.hpp"

#include <chrono>

namespace rvc {

Timer::Timer(ClockFn clock) : clock_{std::move(clock)} {
}

std::int64_t Timer::defaultClock() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
               std::chrono::steady_clock::now().time_since_epoch())
        .count();
}

void Timer::setDuration(std::int64_t durationMs) {
    duration_ = durationMs;
}

void Timer::setCallback(Callback callback) {
    callback_ = std::move(callback);
}

void Timer::start() {
    startTime_ = clock_();
    isRunning_ = true;
    isFired_ = false;
}

void Timer::stop() {
    isRunning_ = false;
}

void Timer::reset() {
    startTime_ = clock_();
    isFired_ = false;
}

bool Timer::isExpired() const {
    return isRunning_ && ((clock_() - startTime_) >= duration_);
}

bool Timer::isRunning() const {
    return isRunning_;
}

void Timer::update() {
    if (isRunning_ && !isFired_ && isExpired()) {
        isFired_ = true;

        if (callback_) {
            callback_();
        }
    }
}

} // namespace rvc