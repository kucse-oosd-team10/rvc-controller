#pragma once

#include "rvc/i_avoid_strategy.hpp"
#include "rvc/types.hpp"

namespace rvc {

class DefaultAvoidStrategy : public IAvoidStrategy {
public:
    Direction decideDirection(bool front, bool left, bool right) override;
    bool needsReverse(bool front, bool left, bool right) override;
};
} // namespace rvc
