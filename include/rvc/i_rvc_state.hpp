#pragma once

namespace rvc {

class RVCController;

class IRVCState {
public:
    IRVCState() = default;
    virtual ~IRVCState() = default;

    IRVCState(const IRVCState&) = delete;
    IRVCState& operator=(const IRVCState&) = delete;
    IRVCState(IRVCState&&) = delete;
    IRVCState& operator=(IRVCState&&) = delete;

    virtual void onEnter(RVCController& ctx) = 0;
    virtual void onExit(RVCController& ctx) = 0;
    virtual void handleObstacle(RVCController& ctx, bool front, bool left, bool right) = 0;
    virtual void handleDust(RVCController& ctx, bool detected) = 0;
    virtual void handlePowerOff(RVCController& ctx) = 0;
};

} // namespace rvc
