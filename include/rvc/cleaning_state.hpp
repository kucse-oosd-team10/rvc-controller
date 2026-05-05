#pragma once

#include "i_rvc_state.hpp"

namespace rvc {
class CleaningState : public IRVCState {
public:
    CleaningState() = default;
    ~CleaningState() override = default;

    void onEnter(RVCController& ctx) override;
    void onExit(RVCController& ctx) override;

    void handleObstacle(RVCController& ctx, bool front, bool left, bool right) override;
    void handleDust(RVCController& ctx, bool detected) override;
    void handlePowerOff(RVCController& ctx) override;
};
} // namespace rvc