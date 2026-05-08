#include "rvc/initializing_state.hpp"

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
    retryCount_ = 0;

    while (retryCount_ < maxRetry) {
        const bool obstacleInitialized = obstacleSensor_.initialize();
        const bool dustInitialized = dustSensor_.initialize();
        const bool motorInitialized = motor_.initialize();
        const bool cleanerInitialized = cleaner_.initialize();

        if (obstacleInitialized && dustInitialized && motorInitialized && cleanerInitialized) {
            ctx.enterCleaning();
            return;
        }
        ++retryCount_;
    }
    ctx.enterError();
}

void InitializingState::onExit(RVCController& /*ctx*/) {
}

void InitializingState::handleObstacle(RVCController& /*ctx*/, bool /*front*/, bool /*left*/,
                                       bool /*right*/) {
}

void InitializingState::handleDust(RVCController& /*ctx*/, bool /*detected*/) {
}

void InitializingState::handlePowerOff(RVCController& ctx) {
    ctx.enterOff();
}

} // namespace rvc
