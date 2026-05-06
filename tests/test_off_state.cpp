#include "rvc/off_state.hpp"
#include "rvc/rvc_controller.hpp"

#include <gtest/gtest.h>

namespace rvc {

// 1. Spy 클래스: onExit가 호출되는지 감시합니다.
class SpyOffState : public OffState {
public:
    void onExit(RVCController& ctx) override {
        OffState::onExit(ctx);
        onExitCalled = true;
    }

    bool onExitCalled{false};
};

// 2. 테스트 픽스처 (Fixture)
class OffStateTest : public ::testing::Test {
protected:
    RVCController controller;
    OffState state;
};

// 3. 테스트 케이스들

TEST_F(OffStateTest, OnEnterHandlesNullManagers) {
    // onEnter 실행 시 예외나 크래시가 없는지 확인합니다.
    EXPECT_NO_THROW(state.onEnter(controller));
}

TEST_F(OffStateTest, HandlePowerOffDoesNotTransition) {
    // Off 상태에서 PowerOff를 눌러도 상태 변화가 없어야 합니다.
    SpyOffState spyState;
    controller.setState(&spyState);
    ASSERT_FALSE(spyState.onExitCalled);

    spyState.handlePowerOff(controller);
    // Off 상태이므로 다시 Off로 갈 필요가 없으므로 onExit는 false여야 함
    EXPECT_FALSE(spyState.onExitCalled);
}

TEST_F(OffStateTest, HandleObstacleDoesNotTransition) {
    // 꺼져 있는 상태(Off)에서는 장애물이 감지되어도 반응하지 않아야 합니다.
    SpyOffState spyState;
    controller.setState(&spyState);

    spyState.handleObstacle(controller, true, true, true);
    EXPECT_FALSE(spyState.onExitCalled);
}

TEST_F(OffStateTest, HandleDustDoesNotTransition) {
    // 꺼져 있는 상태(Off)에서는 먼지가 감지되어도 반응하지 않아야 합니다.
    SpyOffState spyState;
    controller.setState(&spyState);

    spyState.handleDust(controller, true);
    EXPECT_FALSE(spyState.onExitCalled);
}

} // namespace rvc