#pragma once

#include "rvc/types.hpp"

namespace rvc {

class IAvoidStrategy {
public:
    IAvoidStrategy() = default;
    virtual ~IAvoidStrategy() = default;

    IAvoidStrategy(const IAvoidStrategy&) = delete;
    IAvoidStrategy& operator=(const IAvoidStrategy&) = delete;
    IAvoidStrategy(IAvoidStrategy&&) = delete;
    IAvoidStrategy& operator=(IAvoidStrategy&&) = delete;

    virtual Direction decideDirection(bool front, bool left, bool right) = 0;
    virtual bool needsReverse(bool front, bool left, bool right) = 0;
};

} // namespace rvc
