#pragma once

#include "rvc/types.hpp"

namespace rvc {

class ICleaner {
public:
    ICleaner() = default;
    virtual ~ICleaner() = default;

    ICleaner(const ICleaner&) = delete;
    ICleaner& operator=(const ICleaner&) = delete;
    ICleaner(ICleaner&&) = delete;
    ICleaner& operator=(ICleaner&&) = delete;

    virtual bool initialize() = 0;
    virtual void setPower(PowerLevel level) = 0;
};
} // namespace rvc