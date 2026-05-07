#pragma once

#include "rvc/i_rvc_state.hpp"

namespace rvc {

class IObstacleSensor;
class IDustSensor;
class IMotor;
class ICleaner;

class InitializingState : public IRVCState {
public:
    InitializingState(IObstacleSensor& obstacleSensor, IDustSensor& dustSensor, IMotor& motor,
                      ICleaner& cleaner);
    ~InitializingState() override = default;

    InitializingState(const InitializingState&) = delete;
    InitializingState& operator=(const InitializingState&) = delete;
    InitializingState(InitializingState&&) = delete;
    InitializingState& operator=(InitializingState&&) = delete;

    void onEnter(RVCController& ctx) override;
    void onExit(RVCController& ctx) override;
    void handleObstacle(RVCController& ctx, bool front, bool left, bool right) override;
    void handleDust(RVCController& ctx, bool detected) override;
    void handlePowerOff(RVCController& ctx) override;

private:
    IObstacleSensor& obstacleSensor_;
    IDustSensor& dustSensor_;
    IMotor& motor_;
    ICleaner& cleaner_;

    int retryCount_{0};
    static constexpr int maxRetry = 3;
};

} // namespace rvc
