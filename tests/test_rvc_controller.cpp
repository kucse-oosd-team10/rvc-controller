#include "rvc/rvc_controller.hpp"
#include "rvc/i_rvc_state.hpp"
#include <gtest/gtest.h>

namespace {

class MockState : public rvc::IRVCState {
public:
    void onEnter(rvc::RVCController& ctx) override {
        enterCalled = true;
        contextPassed = &ctx;
    }
    void onExit(rvc::RVCController& ctx) override {
        exitCalled = true;
        contextPassed = &ctx;
    }
    void handleObstacle(rvc::RVCController& ctx, bool front, bool left, bool right) override {
        obstacleCalled = true;
        contextPassed = &ctx;
        lastFront = front;
        lastLeft = left;
        lastRight = right;
    }
    void handleDust(rvc::RVCController& ctx, bool detected) override {
        dustCalled = true;
        contextPassed = &ctx;
        lastDust = detected;
    }
    void handlePowerOff(rvc::RVCController& ctx) override {
        powerOffCalled = true;
        contextPassed = &ctx;
    }

    bool enterCalled{false};
    bool exitCalled{false};
    bool obstacleCalled{false};
    bool dustCalled{false};
    bool powerOffCalled{false};
    rvc::RVCController* contextPassed{nullptr};
    bool lastFront{false};
    bool lastLeft{false};
    bool lastRight{false};
    bool lastDust{false};
};

} // namespace

// setState 호출 시 이전 상태의 onExit와 새 상태의 onEnter가 호출되는지 확인
TEST(RVCControllerTest, SetStateCallsEnterAndExit) {
    rvc::RVCController controller;
    MockState state1;
    MockState state2;

    controller.setState(&state1);
    EXPECT_TRUE(state1.enterCalled);
    EXPECT_EQ(state1.contextPassed, &controller);

    controller.setState(&state2);
    EXPECT_TRUE(state1.exitCalled);
    EXPECT_TRUE(state2.enterCalled);
    EXPECT_EQ(state2.contextPassed, &controller);
}

// powerOff 호출 시 현재 상태의 handlePowerOff로 위임되는지 확인
TEST(RVCControllerTest, PowerOffDelegatesToState) {
    rvc::RVCController controller;
    MockState state;
    controller.setState(&state);

    controller.powerOff();
    EXPECT_TRUE(state.powerOffCalled);
    EXPECT_EQ(state.contextPassed, &controller);
}

// 장애물 감지 이벤트가 현재 상태의 handleObstacle로 위임되는지 확인
TEST(RVCControllerTest, ObstacleDetectedDelegatesToState) {
    rvc::RVCController controller;
    MockState state;
    controller.setState(&state);

    controller.onObstacleDetected(true, false, true);
    EXPECT_TRUE(state.obstacleCalled);
    EXPECT_EQ(state.contextPassed, &controller);
    EXPECT_TRUE(state.lastFront);
    EXPECT_FALSE(state.lastLeft);
    EXPECT_TRUE(state.lastRight);
}

// 먼지 감지 이벤트가 현재 상태의 handleDust로 위임되는지 확인
TEST(RVCControllerTest, DustDetectedDelegatesToState) {
    rvc::RVCController controller;
    MockState state;
    controller.setState(&state);

    controller.onDustDetected(true);
    EXPECT_TRUE(state.dustCalled);
    EXPECT_EQ(state.contextPassed, &controller);
    EXPECT_TRUE(state.lastDust);
}

// 초기화 시 각 매니저들이 nullptr로 시작하는지 확인 (현재 구현 기준)
TEST(RVCControllerTest, ManagersAreNullOnInit) {
    rvc::RVCController controller;
    EXPECT_EQ(controller.getMovementManager(), nullptr);
    EXPECT_EQ(controller.getCleaningManager(), nullptr);
    EXPECT_EQ(controller.getObstacleSensorSubject(), nullptr);
    EXPECT_EQ(controller.getDustSensorSubject(), nullptr);
}

// 상태가 없을 때(nullptr) 호출해도 크래시가 나지 않는지 확인
TEST(RVCControllerTest, NoCrashWhenStateIsNull) {
    rvc::RVCController controller;
    EXPECT_NO_THROW(controller.powerOff());
    EXPECT_NO_THROW(controller.onObstacleDetected(true, true, true));
    EXPECT_NO_THROW(controller.onDustDetected(true));
}

// powerOn 호출 확인 (현재 스켈레톤 구현은 아무 동작도 하지 않음)
TEST(RVCControllerTest, PowerOnDoesNothingInSkeleton) {
    rvc::RVCController controller;
    EXPECT_NO_THROW(controller.powerOn());
}
