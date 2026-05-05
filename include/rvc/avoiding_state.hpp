#pragma once

#include "rvc/i_rvc_state.hpp"

#include <iostream>

namespace rvc {
class AvoidingState : public IRVCState {
public:
    AvoidingState() = default;
    ~AvoidingState() override = default;

    AvoidingState(const AvoidingState&) = delete;
    AvoidingState& operator=(const AvoidingState&) = delete;
    AvoidingState(AvoidingState&&) = delete;
    AvoidingState& operator=(AvoidingState&&) = delete;

    void onEnter(RVCController& ctx) override;
    void onExit(RVCController& ctx) override;

    void handleObstacle(RVCController& ctx, bool front, bool left, bool right) override {
        std::cout << "[AvoidingState] handleObstacle을 무시합니다: 회피 동작 우선 수행 중";
    };

    void handleDust(RVCController& ctx, bool detected) override {
        std::cout << "[AvoidingState] handleDust를 무시합니다: 회피 동작 우선 수행 중";
    };

    void handlePowerOff(RVCController& ctx) override;
};
} // namespace rvc