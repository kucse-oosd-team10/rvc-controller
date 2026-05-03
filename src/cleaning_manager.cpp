#include "rvc/cleaning_manager.hpp"

namespace rvc {

CleaningManager::CleaningManager(ICleaner* cleaner, DustSensorSubject* dustSub,
                                 Timer::ClockFn clockFn)
    : cleaner_(cleaner), dustSub_(dustSub), dustTimer_(std::move(clockFn)) {
    dustTimer_.setDuration(powerUpDuration);
    dustTimer_.setCallback([this] {
        onTimerExpired();
    });
}

void CleaningManager::startCleaning() {
    if (cleaner_ == nullptr) {
        return;
    }

    cleaner_->setPower(PowerLevel::NORMAL);
    powerLevel_ = PowerLevel::NORMAL;
}

void CleaningManager::stopCleaning() {
    if (cleaner_ == nullptr) {
        return;
    }

    cleaner_->setPower(PowerLevel::OFF);
    powerLevel_ = PowerLevel::OFF;
    dustTimer_.stop();
}

void CleaningManager::powerUp() {
    if (cleaner_ == nullptr) {
        return;
    }

    cleaner_->setPower(PowerLevel::POWER_UP);
    powerLevel_ = PowerLevel::POWER_UP;
    dustTimer_.start();
}

void CleaningManager::handleDustDetected(bool detected) {
    dustDetected_ = detected;
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

bool CleaningManager::isDustDetected() const {
    return dustDetected_;
}

void CleaningManager::onTimerExpired() {
    if (dustDetected_) { // POWER-UP 유지
        dustTimer_.reset();
        dustTimer_.start();
        return;
    }

    if (cleaner_ != nullptr) {
        cleaner_->setPower(PowerLevel::NORMAL);
    }
    powerLevel_ = PowerLevel::NORMAL;
    dustTimer_.stop();
}

} // namespace rvc
