#include "rvc/initializing_state.hpp"

#include "rvc/cleaning_manager.hpp"
#include "rvc/movement_manager.hpp"
#include "rvc/rvc_controller.hpp"

// #include "rvc/cleaning_state.hpp"

namespace rvc {
void InitializingState::onEnter(RVCController& ctx) {
    while (retryCount_ < MAX_RETRIES_) {
        bool allSuccess = true;

        // Sensor merge 이후 구현 예정입니다.

        if (allSuccess) {
            ctx.setState(nullptr); // CleaningState merge 이후 변경 예정입니다.
            return;
        }

        retryCount_++;
    }

    if (retryCount_ >= MAX_RETRIES_) {
        ctx.setState(nullptr); // OffState merge 후 변경 예정입니다.
    }
}

void InitializingState::onExit(RVCController& ctx) {
    retryCount_ = 0;
}

void InitializingState::handleObstacle(RVCController& ctx, bool front, bool left, bool right) {
    // initializingState 에서 handleObstacle은 무시됩니다.
}

void InitializingState::handleDust(RVCController& ctx, bool detected) {
    CleaningManager* cleaningMgr = ctx.getCleaningManager();
    if (cleaningMgr != nullptr) {
        cleaningMgr->handleDustDetected(detected);
    }
}

void InitializingState::handlePowerOff(RVCController& ctx) {
    MovementManager* movementMgr = ctx.getMovementManager();
    if (movementMgr != nullptr) {
        movementMgr->stop();
    }

    CleaningManager* cleaningMgr = ctx.getCleaningManager();
    if (cleaningMgr != nullptr) {
        cleaningMgr->stopCleaning();
    }

    ctx.setState(nullptr); // offState merge 이후 변경 예정입니다.
}

} // namespace rvc