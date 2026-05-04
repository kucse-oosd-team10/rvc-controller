#include "rvc/off_state.hpp"

#include "rvc/rvc_controller.hpp"

#include <iostream>

namespace rvc {

void OffState::onEnter(RVCController& ctx) {
    std::cout << "off" << std::endl;
}

void OffState::onExit(RVCController& ctx) {
}

void OffState::handleObstacle(RVCController& ctx, bool front, bool left, bool right) {
}

void OffState::handleDust(RVCController& ctx, bool detected) {
}

void OffState::handlePowerOff(RVCController& ctx) {
}
} // namespace rvc