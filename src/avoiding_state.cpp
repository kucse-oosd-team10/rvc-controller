#include "rvc/avoiding_state.hpp"

#include "rvc/i_rvc_state.hpp"
#include "rvc/rvc_controller.hpp"

namespace rvc {
void AvoidingState::onEnter(RVCController& ctx) {
    MovementManager* movementMgr = ctx.getMovementManager();
    if (movementMgr != nullptr) {
        // movementMgr->executeAvoidance(true, false, false);
    }

    // 장애물 회피가 완료된 뒤
    IRVCState* cleaningState = nullptr;
    ctx.setState(cleaningState);
}

void AvoidingState::handlePowerOff(RVCController& ctx) {
    MovementManager* movementMgr = ctx.getMovementManager();
    if (movementMgr != nullptr) {
        // movementMgr->stop();
    }

    CleaningManager* cleaningMgr = ctx.getCleaningManager();
    if (cleaningMgr != nullptr) {
        // cleaningMgr->stopCleaning();
    }

    IRVCState* offState = nullptr;
    ctx.setState(offState);
}
} // namespace rvc