#pragma once

#include "dust_sensor_subject.hpp"
#include "i_cleaner.hpp"
#include "timer.hpp"
#include "types.hpp"

namespace rvc {

class CleaningManager {
public:
    static constexpr int powerUpDuration = 3000; // POWER_UP_DURATION

    explicit CleaningManager(ICleaner& cleaner, DustSensorSubject& dustSub, Timer::ClockFn clockFn);
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
    PowerLevel getPowerLevel() const;
    bool isDustDetected() const;

private:
    ICleaner& cleaner_;
    PowerLevel powerLevel_{0};
    DustSensorSubject& dustSub_;
    Timer dustTimer_;

    bool dustDetected_{false};
    void onTimerExpired();
};

} // namespace rvc
