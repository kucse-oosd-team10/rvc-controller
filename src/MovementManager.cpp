#include "rvc/MovementManager.hpp"

#include "rvc/i_avoid_strategy.hpp"
#include "rvc/i_motor.hpp"
#include "rvc/i_obstacle_sensor.hpp"

namespace rvc {

MovementManager::MovementManager(IMotor* m, IAvoidStrategy* s, IObstacleSensor* o)
    : motor(m), strategy(s), obstacleSensor(o) {
}

void MovementManager::moveForward() {
    motor->move(Direction::FORWARD);
}

void MovementManager::moveBackward() {
    motor->move(Direction::BACKWARD);
}

void MovementManager::turn(Direction direction) {
    motor->move(direction);
}

void MovementManager::stop() {
    motor->move(Direction::STOP);
}

void MovementManager::executeAvoidance(bool front, bool left, bool right) {
    bool leftStatus = left;
    bool rightStatus = right;
    if (strategy->needsReverse(front, left, right)) {
        moveBackward();
        while (leftStatus && rightStatus) {
            leftStatus = obstacleSensor->isLeftDetected();
            rightStatus = obstacleSensor->isRightDetected();
        }
        if (leftStatus && !rightStatus) {
            turn(Direction::RIGHT);
        } else {
            turn(Direction::LEFT);
        }
    } else {
        Direction nextDir = strategy->decideDirection(front, left, right);
        turn(nextDir);
    }
}
} // namespace rvc