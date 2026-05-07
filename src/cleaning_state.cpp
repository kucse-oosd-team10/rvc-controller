#include "rvc/cleaning_state.hpp"

#include "rvc/cleaning_manager.hpp"
#include "rvc/movement_manager.hpp"
#include "rvc/rvc_controller.hpp"

namespace rvc {

void CleaningState::onEnter(RVCController& ctx) {
    MovementManager* movementMgr = ctx.getMovementManager();
    if (movementMgr != nullptr) {
        movementMgr->moveForward();
    }

    CleaningManager* cleaningMgr = ctx.getCleaningManager();
    if (cleaningMgr != nullptr) {
        cleaningMgr->startCleaning();
    }
}

void CleaningState::onExit(RVCController& /*ctx*/) {
}

void CleaningState::handleObstacle(RVCController& ctx, bool front, bool left, bool right) {
    CleaningManager* cleaningMgr = ctx.getCleaningManager();
    if (cleaningMgr != nullptr) {
        cleaningMgr->stopCleaning();
    }

    MovementManager* movementMgr = ctx.getMovementManager();
    if (movementMgr != nullptr) {
        movementMgr->stop();
    }

    ctx.enterAvoiding(front, left, right);
}

void CleaningState::handleDust(RVCController& ctx, bool detected) {
    CleaningManager* cleaningMgr = ctx.getCleaningManager();
    if (cleaningMgr != nullptr) {
        cleaningMgr->handleDustDetected(detected);
    }
}

void CleaningState::handlePowerOff(RVCController& ctx) {
    CleaningManager* cleaningMgr = ctx.getCleaningManager();
    if (cleaningMgr != nullptr) {
        cleaningMgr->stopCleaning();
    }

    MovementManager* movementMgr = ctx.getMovementManager();
    if (movementMgr != nullptr) {
        movementMgr->stop();
    }

    ctx.enterOff();
}

} // namespace rvc
