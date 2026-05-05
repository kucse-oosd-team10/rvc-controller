#include "rvc/cleaning_state.hpp"

#include "rvc/i_rvc_state.hpp"
#include "rvc/rvc_controller.hpp"

namespace rvc {

void CleaningState::onEnter(RVCController& ctx) {
    MovementManager* movementMgr = ctx.getMovementManager();
    if (movementMgr != nullptr) {
        // movementMgr->moveForward();
    }

    CleaningManager* cleaningMgr = ctx.getCleaningManager();
    if (cleaningMgr != nullptr) {
        // cleaningMgr->startCleaning();
    }
}

void CleaningState::onExit(RVCController& ctx) {
    // 구현 예정
}

void CleaningState::handlePowerOff(RVCController& ctx) {
    CleaningManager* cleaningMgr = ctx.getCleaningManager();
    if (cleaningMgr != nullptr) {
        // cleaningMgr->stopCleaning();
    }

    MovementManager* movementMgr = ctx.getMovementManager();
    if (movementMgr != nullptr) {
        // movementMgr->stop();
    }

    IRVCState* offState = nullptr; // 추후 변경 예정
    ctx.setState(offState);
}

void CleaningState::handleObstacle(RVCController& ctx, bool front, bool left, bool right) {
    CleaningManager* cleaningMgr = ctx.getCleaningManager();
    if (cleaningMgr != nullptr) {
        // cleaningMgr->stopCleaning();
    }

    MovementManager* movementMgr = ctx.getMovementManager();
    if (movementMgr != nullptr) {
        // moveMentMgr->stop();
    }

    IRVCState* avoidingState = nullptr; // 추후 변경 예정
    ctx.setState(avoidingState);
}

void CleaningState::handleDust(RVCController& ctx, bool detected) {
    CleaningManager* cleaningMgr = ctx.getCleaningManager();
    if (cleaningMgr != nullptr) {
        // cleaningMgr->handleDustDetected(detected);
    }
}

} // namespace rvc