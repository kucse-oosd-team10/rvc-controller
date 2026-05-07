#include "rvc/initializing_state.hpp"

#include "rvc/cleaning_state.hpp"
#include "rvc/error_state.hpp"
#include "rvc/i_cleaner.hpp"
#include "rvc/i_dust_sensor.hpp"
#include "rvc/i_motor.hpp"
#include "rvc/i_obstacle_sensor.hpp"
#include "rvc/rvc_controller.hpp"

namespace rvc {

InitializingState::InitializingState(IObstacleSensor& obstacleSensor, IDustSensor& dustSensor,
                                     IMotor& motor, ICleaner& cleaner)
    : obstacleSensor_{obstacleSensor}, dustSensor_{dustSensor}, motor_{motor}, cleaner_{cleaner} {
}

void InitializingState::onEnter(RVCController& ctx) {
    while (retryCount_ < maxRetry) {
        if (obstacleSensor_.initialize() && dustSensor_.initialize() && motor_.initialize() &&
            cleaner_.initialize()) {
            // TODO: owning 슬롯으로 교체
            ctx.setState(new CleaningState());
            return;
        }
        ++retryCount_;
    }
    // TODO: owning 슬롯으로 교체
    ctx.setState(new ErrorState());
}

void InitializingState::onExit(RVCController& /*ctx*/) {}

void InitializingState::handleObstacle(RVCController& /*ctx*/, bool /*front*/, bool /*left*/,
                                       bool /*right*/) {}

void InitializingState::handleDust(RVCController& /*ctx*/, bool /*detected*/) {}

void InitializingState::handlePowerOff(RVCController& /*ctx*/) {}

} // namespace rvc
