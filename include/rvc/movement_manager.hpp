#pragma once

#include "rvc/types.hpp"

namespace rvc {
class IMotor;
class IAvoidStrategy;

class MovementManager {
public:
    MovementManager(IMotor& motor_, IAvoidStrategy& strategy_);
    ~MovementManager() = default;

    MovementManager(const MovementManager&) = delete;
    MovementManager& operator=(const MovementManager&) = delete;
    MovementManager(MovementManager&&) = delete;
    MovementManager& operator=(MovementManager&&) = delete;

    void moveForward();
    void moveBackward();
    void turn(Direction direction);
    void stop();
    void executeAvoidance(bool front, bool left, bool right);
    bool needsReverse(bool front, bool left, bool right) const;

private:
    IMotor& motor_;
    IAvoidStrategy& strategy_;
    Direction LastDirection_{Direction::STOP};
};
} // namespace rvc