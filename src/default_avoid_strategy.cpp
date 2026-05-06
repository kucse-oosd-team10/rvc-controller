#include "rvc/default_avoid_strategy.hpp"

namespace rvc {
Direction DefaultAvoidStrategy::decideDirection(bool front, bool left, bool right) {
    if (front && left && right) {
        return Direction::BACKWARD;
    }

    if (front && left) {
        return Direction::RIGHT;
    }

    if (front) {
        return Direction::LEFT;
    }
    return Direction::FORWARD;
}

bool DefaultAvoidStrategy::needsReverse(bool front, bool left, bool right) {
    return (front && left && right);
}
} // namespace rvc