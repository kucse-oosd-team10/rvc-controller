#include "rvc/avoiding_state.hpp"

#include "rvc/cleaning_manager.hpp"
#include "rvc/i_obstacle_sensor.hpp"
#include "rvc/movement_manager.hpp"
#include "rvc/off_state.hpp"
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

        // 후진 완료 후 모든 센서 재확인
        front = sensor->isFrontDetected();
        left = sensor->isLeftDetected();
        right = sensor->isRightDetected();
    }

    mm->executeAvoidance(front, left, right);

    // CleaningState 머지 후 setState(&cleaningState) 로 교체
    ctx.setState(nullptr);
}

void AvoidingState::onExit(RVCController& /*ctx*/) {
}

void AvoidingState::handleObstacle(RVCController& /*ctx*/, bool /*front*/, bool /*left*/,
                                   bool /*right*/) {
    // 회피 동작 우선 — 새 obstacle 이벤트 무시
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

    static OffState offState;
    ctx.setState(&offState);
}

} // namespace rvc
