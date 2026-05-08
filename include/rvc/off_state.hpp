#pragma once

#include "rvc/i_rvc_state.hpp"

namespace rvc {
class OffState : public IRVCState {
public:
    OffState() = default;
    ~OffState() override = default;

    OffState(const OffState&) = delete;
    OffState& operator=(const OffState&) = delete;
    OffState(OffState&&) = delete;
    OffState& operator=(OffState&&) = delete;

    void onEnter(RVCController& ctx) override;
    void onExit(RVCController& ctx) override;

    void handleObstacle(RVCController& ctx, bool front, bool left, bool right) override;
    void handleDust(RVCController& ctx, bool detected) override;
    void handlePowerOff(RVCController& ctx) override;
};
} // namespace rvc