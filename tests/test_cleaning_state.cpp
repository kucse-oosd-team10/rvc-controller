#include "rvc/cleaning_state.hpp"
#include "rvc/rvc_controller.hpp"

#include <gtest/gtest.h>

namespace rvc {

class SpyCleaningState : public CleaningState {
public:
    void onExit(RVCController& ctx) override {
        CleaningState::onExit(ctx);
        onExitCalled = true;
    }

    bool onExitCalled{false};
};

class CleaningStateTest : public ::testing::Test {
protected:
    RVCController controller;
    CleaningState state;
};

TEST_F(CleaningStateTest, OnEnterHandlesNullManagers) {
    // onEnter 호출 시 크래시가 나지 않아야 함을 확인합니다.
    EXPECT_NO_THROW(state.onEnter(controller));
}

TEST_F(CleaningStateTest, HandlePowerOffTransitionsState) {
    // handlePowerOff 호출 시 setState가 호출되어 현재 상태의 onExit가 실행되는지 확인합니다.
    SpyCleaningState spyState;
    controller.setState(&spyState);
    ASSERT_FALSE(spyState.onExitCalled);

    spyState.handlePowerOff(controller);
    EXPECT_TRUE(spyState.onExitCalled);
}

TEST_F(CleaningStateTest, HandleObstacleTransitionsState) {
    // handleObstacle 호출 시 setState가 호출되어 현재 상태의 onExit가 실행되는지 확인합니다.
    SpyCleaningState spyState;
    controller.setState(&spyState);
    ASSERT_FALSE(spyState.onExitCalled);

    // 장애물 감지 시 (전방 장애물)
    spyState.handleObstacle(controller, true, false, false);
    EXPECT_TRUE(spyState.onExitCalled);
}

TEST_F(CleaningStateTest, HandleDustDoesNotTransitionState) {
    // handleDust 호출 시에는 상태 전환이 일어나지 않아야 함을 확인합니다.
    SpyCleaningState spyState;
    controller.setState(&spyState);
    ASSERT_FALSE(spyState.onExitCalled);

    // 먼지 감지 시
    spyState.handleDust(controller, true);
    EXPECT_FALSE(spyState.onExitCalled);
}

} // namespace rvc
