#include "rvc/error_state.hpp"

#include "rvc/cleaning_manager.hpp"
#include "rvc/movement_manager.hpp"
#include "rvc/off_state.hpp"
#include "rvc/rvc_controller.hpp"

#include <iostream>

namespace rvc {

void ErrorState::onEnter(RVCController& ctx) {
    std::cout << "Error: Init Failed\n";

    MovementManager* movementMgr = ctx.getMovementManager();
    if (movementMgr != nullptr) {
        movementMgr->stop();
    }

    CleaningManager* cleaningMgr = ctx.getCleaningManager();
    if (cleaningMgr != nullptr) {
        cleaningMgr->stopCleaning();
    }
}

void ErrorState::onExit(RVCController& /*ctx*/) {
}

void ErrorState::handleObstacle(RVCController& /*ctx*/, bool /*front*/, bool /*left*/,
                                bool /*right*/) {
}

void ErrorState::handleDust(RVCController& /*ctx*/, bool /*detected*/) {
}

void ErrorState::handlePowerOff(RVCController& ctx) {
    static OffState offState;
    ctx.setState(&offState);
}

} // namespace rvc
