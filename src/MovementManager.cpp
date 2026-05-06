#include "rvc/MovementManager.hpp"

#include "rvc/i_avoid_strategy.hpp"
#include "rvc/i_motor.hpp"
#include "rvc/i_obstacle_sensor.hpp"

namespace rvc {

MovementManager::MovementManager(IMotor& motor_, IAvoidStrategy& strategy_)
    : motor_(&motor_), strategy_(&strategy_) {
}

void MovementManager::moveForward() {
    if (LastDirection_ != Direction::FORWARD) {
        motor_->move(Direction::FORWARD);
        LastDirection_ = Direction::FORWARD;
    }
}

void MovementManager::moveBackward() {
    if (LastDirection_ != Direction::BACKWARD) {
        motor_->move(Direction::BACKWARD);
        LastDirection_ = Direction::BACKWARD;
    }
}

void MovementManager::turn(Direction direction) {
    if (LastDirection_ != direction) {
        motor_->move(direction);
        LastDirection_ = direction;
    }
}

void MovementManager::stop() {
    if (LastDirection_ != Direction::STOP) {
        motor_->move(Direction::STOP);
        LastDirection_ = Direction::STOP;
    }
}

void MovementManager::executeAvoidance(bool front, bool left, bool right) {
    if (strategy_->needsReverse(front, left, right)) {
        // 후진과 센서 확인은 AvoidingState에서 처리
        Direction nextDir = strategy_->decideDirection(front, left, right);
        turn(nextDir);
    } else {
        Direction nextDir = strategy_->decideDirection(front, left, right);
        if (LastDirection_ != nextDir) {
            turn(nextDir);
        }
    }
}
} // namespace rvc