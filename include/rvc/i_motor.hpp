#pragma once

#include "rvc/types.hpp"

namespace rvc {

class IMotor {
public:
    IMotor() = default;
    virtual ~IMotor() = default;

    IMotor(const IMotor&) = delete;
    IMotor& operator=(const IMotor&) = delete;
    IMotor(IMotor&&) = delete;
    IMotor& operator=(IMotor&&) = delete;

    virtual bool initialize() = 0;
    virtual void move(Direction direction) = 0;
};
} // namespace rvc