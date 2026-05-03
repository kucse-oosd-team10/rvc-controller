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
    motor_->move(Direction::FORWARD);
}

void MovementManager::moveBackward() {
    motor_->move(Direction::BACKWARD);
}

void MovementManager::turn(Direction direction) {
    motor_->move(direction);
}

void MovementManager::stop() {
    motor_->move(Direction::STOP);
}

void MovementManager::executeAvoidance(bool front, bool left, bool right) {
    bool leftStatus = left;
    bool rightStatus = right;
    if (strategy_->needsReverse(front, left, right)) {
        moveBackward();
        while (leftStatus && rightStatus) {
            leftStatus = obstacleSensor_->isLeftDetected();
            rightStatus = obstacleSensor_->isRightDetected();
        }
        if (leftStatus && !rightStatus) {
            turn(Direction::RIGHT);
        } else {
            turn(Direction::LEFT);
        }
    } else {
        Direction nextDir = strategy_->decideDirection(front, left, right);
        turn(nextDir);
    }
}
} // namespace rvc