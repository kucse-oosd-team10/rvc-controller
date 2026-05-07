#pragma once

#include "rvc/i_rvc_state.hpp"

namespace rvc {

class AvoidingState : public IRVCState {
public:
    AvoidingState(bool front, bool left, bool right);
    ~AvoidingState() override = default;

    AvoidingState(const AvoidingState&) = delete;
    AvoidingState& operator=(const AvoidingState&) = delete;
    AvoidingState(AvoidingState&&) = delete;
    AvoidingState& operator=(AvoidingState&&) = delete;

    void onEnter(RVCController& ctx) override;
    void onExit(RVCController& ctx) override;
    void handleObstacle(RVCController& ctx, bool front, bool left, bool right) override;
    void handleDust(RVCController& ctx, bool detected) override;
    void handlePowerOff(RVCController& ctx) override;

private:
    bool front_;
    bool left_;
    bool right_;
};

} // namespace rvc
