#include "rvc/initializing_state.hpp"

#include "rvc/rvc_controller.hpp"

// #include "rvc/cleaning_state.hpp"

namespace rvc {
void InitializingState::onEnter(RVCController& ctx) {
    auto* movementManager = ctx.getMovementManager();
    auto* CleaningManager = ctx.getCleaningManager();
    auto* ObstacleSensorSubject = ctx.getObstacleSensorSubject();
    auto* DustSensorSubject = ctx.getDustSensorSubject();
}

void InitializingState::onExit(RVCController& ctx) {
}

void InitializingState::handleObstacle(RVCController& ctx, bool front, bool left, bool right) {
}

void InitializingState::handleDust(RVCController& ctx, bool detected) {
}

void InitializingState::handlePowerOff(RVCController& ctx) {
}

} // namespace rvc