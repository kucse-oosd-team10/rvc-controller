#include "rvc/rvc_controller.hpp"

#include "rvc/i_rvc_state.hpp"

namespace rvc {

RVCController::RVCController() = default;
RVCController::~RVCController() = default;

void RVCController::powerOn() const {
    // TO BE IMPLEMENTED
    // Skeleton Code 단계라 구현되지 않았습니다.
    // 임시로 const로 수정하였습니다.
}

void RVCController::powerOff() {
    if (currentState_ != nullptr) {
        currentState_->handlePowerOff(*this);
    }
}

void RVCController::onObstacleDetected(bool front, bool left, bool right) {
    if (currentState_ != nullptr) {
        currentState_->handleObstacle(*this, front, left, right);
    }
}

void RVCController::onDustDetected(bool detected) {
    if (currentState_ != nullptr) {
        currentState_->handleDust(*this, detected);
    }
}

void RVCController::setState(IRVCState* state) {
    if (currentState_ != nullptr) {
        currentState_->onExit(*this);
    }
    currentState_ = state;
    if (currentState_ != nullptr) {
        currentState_->onEnter(*this);
    }
}

MovementManager* RVCController::getMovementManager() {
    return movementMgr_;
}

CleaningManager* RVCController::getCleaningManager() {
    return cleaningMgr_;
}

ObstacleSensorSubject* RVCController::getObstacleSensorSubject() {
    return obstacleSub_;
}

DustSensorSubject* RVCController::getDustSensorSubject() {
    return dustSub_;
}

} // namespace rvc
