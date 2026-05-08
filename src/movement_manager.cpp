#include "rvc/movement_manager.hpp"

#include "rvc/i_avoid_strategy.hpp"
#include "rvc/i_motor.hpp"

namespace rvc {

MovementManager::MovementManager(IMotor& motor_, IAvoidStrategy& strategy_)
    : motor_(motor_), strategy_(strategy_) {
}

void MovementManager::moveForward() {
    if (LastDirection_ != Direction::FORWARD) {
        motor_.move(Direction::FORWARD);
        LastDirection_ = Direction::FORWARD;
    }
}

void MovementManager::moveBackward() {
    if (LastDirection_ != Direction::BACKWARD) {
        motor_.move(Direction::BACKWARD);
        LastDirection_ = Direction::BACKWARD;
    }
}

void MovementManager::turn(Direction direction) {
    if (LastDirection_ != direction) {
        motor_.move(direction);
        LastDirection_ = direction;
    }
}

void MovementManager::stop() {
    if (LastDirection_ != Direction::STOP) {
        motor_.move(Direction::STOP);
        LastDirection_ = Direction::STOP;
    }
}

void MovementManager::executeAvoidance(bool front, bool left, bool right) {
    Direction nextDir = strategy_.decideDirection(front, left, right);
    turn(nextDir);
}

bool MovementManager::needsReverse(bool front, bool left, bool right) const {
    return strategy_.needsReverse(front, left, right);
}

} // namespace rvc