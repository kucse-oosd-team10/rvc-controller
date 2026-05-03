#pragma once

#include "rvc/types.hpp"

namespace rvc {
class IMotor;
class IAvoidStrategy;
class IObstacleSensor;

class MovementManager {
public:
    MovementManager(IMotor* m, IAvoidStrategy* s, IObstacleSensor* o);

    void moveForward();
    void moveBackward();
    void turn(Direction direction);
    void stop();
    void executeAvoidance(bool front, bool left, bool right);

private:
    IMotor* motor;
    IAvoidStrategy* strategy;
    IObstacleSensor* obstacleSensor;
};
} // namespace rvc