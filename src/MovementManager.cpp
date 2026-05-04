#include "rvc/MovementManager.hpp"

#include "rvc/i_avoid_strategy.hpp"
#include "rvc/i_motor.hpp"
#include "rvc/i_obstacle_sensor.hpp"

namespace rvc {

MovementManager::MovementManager(IMotor* motor_, IAvoidStrategy* strategy_,
                                 IObstacleSensor* obstacleSensor_)
    : motor_(motor_), strategy_(strategy_), obstacleSensor_(obstacleSensor_) {
}

void MovementManager::moveForward() {
    if (motor_ == nullptr) {
        return;
    }
    if (LastDirection_ != Direction::FORWARD) {
        motor_->move(Direction::FORWARD);
        LastDirection_ = Direction::FORWARD;
    }
}

void MovementManager::moveBackward() {
    if (motor_ == nullptr) {
        return;
    }
    if (LastDirection_ != Direction::BACKWARD) {
        motor_->move(Direction::BACKWARD);
        LastDirection_ = Direction::BACKWARD;
    }
}

void MovementManager::turn(Direction direction) {
    if (motor_ == nullptr) {
        return;
    }
    if (LastDirection_ != direction) {
        motor_->move(direction);
        LastDirection_ = direction;
    }
}

void MovementManager::stop() {
    if (motor_ == nullptr) {
        return;
    }
    if (LastDirection_ != Direction::STOP) {
        motor_->move(Direction::STOP);
        LastDirection_ = Direction::STOP;
    }
}

void MovementManager::executeAvoidance(bool front, bool left, bool right) {
    bool leftStatus = left;
    bool rightStatus = right;
    if (strategy_->needsReverse(front, left, right)) {
        if (LastDirection_ != Direction::BACKWARD) {
            moveBackward();
        }
        while (leftStatus && rightStatus) {
            leftStatus = obstacleSensor_->isLeftDetected();
            rightStatus = obstacleSensor_->isRightDetected();
        }
        bool currentFront = obstacleSensor_->isFrontDetected();
        bool currentLeft = obstacleSensor_->isLeftDetected();
        bool currentRight = obstacleSensor_->isRightDetected();
        Direction nextDir = strategy_->decideDirection(currentFront, currentLeft, currentRight);
        turn(nextDir);
    } else {
        Direction nextDir = strategy_->decideDirection(front, left, right);
        if (LastDirection_ != nextDir) {
            turn(nextDir);
        }
    }
}
} // namespace rvc