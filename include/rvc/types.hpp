#pragma once

#include <cstdint>

namespace rvc {

enum class Direction : std::uint8_t {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    STOP
};

enum class PowerLevel : std::uint8_t {
    OFF,
    NORMAL,
    POWER_UP
};

} // namespace rvc
