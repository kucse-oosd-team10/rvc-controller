#include "rvc/controller.hpp"

namespace rvc {

Controller::Controller() = default;

void Controller::tick() {
    // All sensors blocked: stop temporarily
    if (frontSensor_ && leftSensor_ && rightSensor_) {
        direction_ = Direction::Backward;
        stopped_ = true;
    } else if (frontSensor_ && leftSensor_) {
        direction_ = Direction::Right;
        stopped_ = false;
    } else if (frontSensor_ && rightSensor_) {
        direction_ = Direction::Left;
        stopped_ = false;
    } else if (frontSensor_) {
        direction_ = Direction::Left;
        stopped_ = false;
    } else {
        direction_ = Direction::Forward;
        stopped_ = false;
    }

    cleanMode_ = dustSensor_ ? CleanMode::PowerUp : CleanMode::On;

    // Reset sensors after processing — requires fresh input each tick
    resetSensors();
}

Direction Controller::getDirection() const {
    return direction_;
}

CleanMode Controller::getCleanMode() const {
    return cleanMode_;
}

void Controller::setFrontSensor(bool detected) {
    frontSensor_ = detected;
}

void Controller::setLeftSensor(bool detected) {
    leftSensor_ = detected;
}

void Controller::setRightSensor(bool detected) {
    rightSensor_ = detected;
}

void Controller::setDustSensor(bool detected) {
    dustSensor_ = detected;
}

bool Controller::isStopped() const {
    return stopped_;
}

void Controller::resetSensors() {
    frontSensor_ = false;
    leftSensor_ = false;
    rightSensor_ = false;
    dustSensor_ = false;
}

} // namespace rvc