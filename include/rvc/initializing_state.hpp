#pragma once

#include "rvc/i_rvc_state.hpp"

namespace rvc {
class InitializingState : public IRVCState {
public:
    InitializingState() = default;
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
    int retryCount_{0};
    const int MAX_RETRIES_{3};
};
}