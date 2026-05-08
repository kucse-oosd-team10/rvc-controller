#pragma once

#include "rvc/types.hpp"

namespace rvc {

class IDustSensor {
public:
    IDustSensor() = default;
    virtual ~IDustSensor() = default;

    IDustSensor(const IDustSensor&) = delete;
    IDustSensor& operator=(const IDustSensor&) = delete;
    IDustSensor(IDustSensor&&) = delete;
    IDustSensor& operator=(IDustSensor&&) = delete;

    virtual bool initialize() = 0;
    virtual bool isDustDetected() = 0;
};
} // namespace rvc