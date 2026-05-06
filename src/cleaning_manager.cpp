#include "rvc/cleaning_manager.hpp"

namespace rvc {

CleaningManager::CleaningManager(ICleaner& cleaner, Timer::ClockFn clockFn)
    : cleaner_(cleaner), dustTimer_(std::move(clockFn)) {
    dustTimer_.setDuration(powerUpDuration);
    dustTimer_.setCallback([this] {
        onTimerExpired();
    });
}

void CleaningManager::startCleaning() {
    if (latestDustDetected_) {
        powerUp();
        return;
    }

    cleaner_.setPower(PowerLevel::NORMAL);
    powerLevel_ = PowerLevel::NORMAL;
}

void CleaningManager::stopCleaning() {
    cleaner_.setPower(PowerLevel::OFF);
    powerLevel_ = PowerLevel::OFF;
    dustTimer_.stop();
}

void CleaningManager::powerUp() {
    cleaner_.setPower(PowerLevel::POWER_UP);
    powerLevel_ = PowerLevel::POWER_UP;
    dustTimer_.start();
}

void CleaningManager::handleDustDetected(bool detected) {
    latestDustDetected_ = detected;
    if (detected && (powerLevel_ != PowerLevel::POWER_UP)) {
        powerUp();
    }
}

void CleaningManager::update() {
    dustTimer_.update();
}

PowerLevel CleaningManager::getPowerLevel() const {
    return powerLevel_;
}

bool CleaningManager::getLatestDustDetected() const {
    return latestDustDetected_;
}

void CleaningManager::onTimerExpired() {
    if (latestDustDetected_) { // POWER-UP 유지
        dustTimer_.reset();
        dustTimer_.start();
        return;
    }

    cleaner_.setPower(PowerLevel::NORMAL);
    powerLevel_ = PowerLevel::NORMAL;
    dustTimer_.stop();
}

} // namespace rvc
