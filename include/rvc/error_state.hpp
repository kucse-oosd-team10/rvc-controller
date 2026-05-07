#pragma once

#include "rvc/i_rvc_state.hpp"

namespace rvc {

class ErrorState : public IRVCState {
public:
    ErrorState() = default;
    ~ErrorState() override = default;

    ErrorState(const ErrorState&) = delete;
    ErrorState& operator=(const ErrorState&) = delete;
    ErrorState(ErrorState&&) = delete;
    ErrorState& operator=(ErrorState&&) = delete;

    void onEnter(RVCController& ctx) override;
    void onExit(RVCController& ctx) override;
    void handleObstacle(RVCController& ctx, bool front, bool left, bool right) override;
    void handleDust(RVCController& ctx, bool detected) override;
    void handlePowerOff(RVCController& ctx) override;
};

} // namespace rvc
