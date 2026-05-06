#pragma once

#include "i_cleaner.hpp"
#include "timer.hpp"
#include "types.hpp"

namespace rvc {

class CleaningManager {
public:
    static constexpr int powerUpDuration = 3000; // POWER_UP_DURATION

    explicit CleaningManager(ICleaner& cleaner, Timer::ClockFn clockFn);
    ~CleaningManager() = default;

    CleaningManager(const CleaningManager&) = delete;
    CleaningManager& operator=(const CleaningManager&) = delete;
    CleaningManager(CleaningManager&&) = delete;
    CleaningManager& operator=(CleaningManager&&) = delete;

    void startCleaning();
    void stopCleaning();
    void powerUp();
    void handleDustDetected(bool detected);
    void update();
    [[nodiscard]] PowerLevel getPowerLevel() const;
    [[nodiscard]] bool getLatestDustDetected() const;

private:
    ICleaner& cleaner_;
    PowerLevel powerLevel_{0};
    Timer dustTimer_;

    bool latestDustDetected_{false};
    void onTimerExpired();
};

} // namespace rvc
