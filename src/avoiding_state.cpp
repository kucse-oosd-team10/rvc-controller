#include "rvc/avoiding_state.hpp"

#include "rvc/cleaning_manager.hpp"
#include "rvc/i_obstacle_sensor.hpp"
#include "rvc/movement_manager.hpp"
#include "rvc/rvc_controller.hpp"

namespace rvc {

AvoidingState::AvoidingState(bool front, bool left, bool right)
    : front_{front}, left_{left}, right_{right} {
}

void AvoidingState::onEnter(RVCController& ctx) {
    MovementManager* mm = ctx.getMovementManager();
    if (mm == nullptr) {
        return;
    }

    bool front = front_;
    bool left = left_;
    bool right = right_;

    if (mm->needsReverse(front, left, right)) {
        IObstacleSensor* sensor = ctx.getObstacleSensor();
        if (sensor == nullptr) {
            return;
        }
        do {
            mm->moveBackward();
            left = sensor->isLeftDetected();
            right = sensor->isRightDetected();
        } while (left && right);

        front = sensor->isFrontDetected();
        left = sensor->isLeftDetected();
        right = sensor->isRightDetected();
    }

    mm->executeAvoidance(front, left, right);

    ctx.enterCleaning();
}

void AvoidingState::onExit(RVCController& /*ctx*/) {
}

void AvoidingState::handleObstacle(RVCController& /*ctx*/, bool /*front*/, bool /*left*/,
                                   bool /*right*/) {
}

void AvoidingState::handleDust(RVCController& ctx, bool detected) {
    CleaningManager* cm = ctx.getCleaningManager();
    if (cm != nullptr) {
        cm->handleDustDetected(detected);
    }
}

void AvoidingState::handlePowerOff(RVCController& ctx) {
    MovementManager* mm = ctx.getMovementManager();
    if (mm != nullptr) {
        mm->stop();
    }

    CleaningManager* cm = ctx.getCleaningManager();
    if (cm != nullptr) {
        cm->stopCleaning();
    }

    ctx.enterOff();
}

} // namespace rvc
