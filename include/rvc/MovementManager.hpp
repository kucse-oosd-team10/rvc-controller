#pragma once

#include "rvc/types.hpp"

namespace rvc {
class IMotor;
class IAvoidStrategy;
class IObstacleSensor;

class MovementManager {
public:
    MovementManager(IMotor* motor_, IAvoidStrategy* strategy_, IObstacleSensor* obstacleSensor_);

    void moveForward();
    void moveBackward();
    void turn(Direction direction);
    void stop();
    void executeAvoidance(bool front, bool left, bool right);

private:
    IMotor* motor_;
    IAvoidStrategy* strategy_;
    IObstacleSensor* obstacleSensor_;
};
} // namespace rvc