#pragma once

#include "rvc/types.hpp"

namespace rvc {

class IObstacleSensor {
public:
    IObstacleSensor() = default;
    virtual ~IObstacleSensor() = default;

    IObstacleSensor(const IObstacleSensor&) = delete;
    IObstacleSensor& operator=(const IObstacleSensor&) = delete;
    IObstacleSensor(IObstacleSensor&&) = delete;
    IObstacleSensor& operator=(IObstacleSensor&&) = delete;

    virtual bool initialize() = 0;
    virtual bool isFrontDetected() = 0;
    virtual bool isLeftDetected() = 0;
    virtual bool isRightDetected() = 0;
};
} // namespace rvc