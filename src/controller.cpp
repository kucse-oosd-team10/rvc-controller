#include "rvc/controller.hpp"

namespace rvc {

Controller::Controller() = default;

void Controller::tick() {
    // Basic obstacle avoidance logic from requirements
    if (frontSensor_ && leftSensor_ && rightSensor_) {
        direction_ = Direction::Backward;
    } else if (frontSensor_ && leftSensor_) {
        direction_ = Direction::Right;
    } else if (frontSensor_ && rightSensor_) {
        direction_ = Direction::Left;
    } else if (frontSensor_) {
        direction_ = Direction::Left; // default: turn left
    } else {
        direction_ = Direction::Forward;
    }

    // Dust detection: power up cleaning
    cleanMode_ = dustSensor_ ? CleanMode::PowerUp : CleanMode::On;
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

} // namespace rvc