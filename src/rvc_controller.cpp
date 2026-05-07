#include "rvc/rvc_controller.hpp"

#include "rvc/avoiding_state.hpp"
#include "rvc/cleaning_manager.hpp"
#include "rvc/dust_sensor_subject.hpp"
#include "rvc/i_rvc_state.hpp"
#include "rvc/obstacle_sensor_subject.hpp"

#include <iostream>

namespace rvc {

RVCController::RVCController(IObstacleSensor& obstacleSensor, IDustSensor& dustSensor,
                             IMotor& motor, ICleaner& cleaner, MovementManager& movementMgr,
                             CleaningManager& cleaningMgr, ObstacleSensorSubject& obstacleSub,
                             DustSensorSubject& dustSub)
    : obstacleSensor_{&obstacleSensor}, dustSensor_{&dustSensor}, motor_{&motor},
      cleaner_{&cleaner}, movementMgr_{&movementMgr}, cleaningMgr_{&cleaningMgr},
      obstacleSub_{&obstacleSub}, dustSub_{&dustSub},
      initializingState_{obstacleSensor, dustSensor, motor, cleaner}, currentState_{&offState_} {
}

RVCController::~RVCController() = default;

void RVCController::powerOn() {
    enterInitializing();
    if (dynamic_cast<InitializingState*>(currentState_) != nullptr) {
        return;
    }
    if (dynamic_cast<ErrorState*>(currentState_) != nullptr) {
        return;
    }
    obstacleSub_->attach(this);
    dustSub_->attach(this);
    std::cout << "Ready" << '\n';
}

void RVCController::powerOff() {
    if (currentState_ != nullptr) {
        currentState_->handlePowerOff(*this);
    }
    obstacleSub_->detach(this);
    dustSub_->detach(this);
    std::cout << "Off" << '\n';
}

void RVCController::tick() {
    obstacleSub_->poll();
    dustSub_->poll();
    cleaningMgr_->update();
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

IRVCState* RVCController::getCurrentState() const {
    return currentState_;
}

void RVCController::enterOff() {
    setState(&offState_);
}

void RVCController::enterInitializing() {
    setState(&initializingState_);
}

void RVCController::enterCleaning() {
    setState(&cleaningState_);
}

void RVCController::enterError() {
    setState(&errorState_);
}

void RVCController::enterAvoiding(bool front, bool left, bool right) {
    currentAvoiding_ = std::make_unique<AvoidingState>(front, left, right);
    setState(currentAvoiding_.get());
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

IObstacleSensor* RVCController::getObstacleSensor() {
    return obstacleSensor_;
}

} // namespace rvc
