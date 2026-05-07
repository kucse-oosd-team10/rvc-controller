#include "rvc/cleaning_manager.hpp"
#include "rvc/cleaning_state.hpp"
#include "rvc/dust_sensor_subject.hpp"
#include "rvc/i_avoid_strategy.hpp"
#include "rvc/i_cleaner.hpp"
#include "rvc/i_dust_sensor.hpp"
#include "rvc/i_motor.hpp"
#include "rvc/i_obstacle_sensor.hpp"
#include "rvc/i_rvc_state.hpp"
#include "rvc/movement_manager.hpp"
#include "rvc/obstacle_sensor_subject.hpp"
#include "rvc/off_state.hpp"
#include "rvc/rvc_controller.hpp"
#include "rvc/types.hpp"

#include <cstdint>
#include <functional>
#include <sstream>

#include <gtest/gtest.h>

namespace {

class FakeMotor : public rvc::IMotor {
public:
    bool initialize() override {
        return true;
    }

    void move(rvc::Direction /*direction*/) override {
    }
};

class FakeCleaner : public rvc::ICleaner {
public:
    bool initialize() override {
        return true;
    }

    void setPower(rvc::PowerLevel /*level*/) override {
    }
};

class FakeObstacleSensor : public rvc::IObstacleSensor {
public:
    bool initialize() override {
        return true;
    }

    bool isFrontDetected() override {
        return false;
    }

    bool isLeftDetected() override {
        return false;
    }

    bool isRightDetected() override {
        return false;
    }
};

class FakeDustSensor : public rvc::IDustSensor {
public:
    bool initialize() override {
        return true;
    }

    bool isDustDetected() override {
        return false;
    }
};

class FakeAvoidStrategy : public rvc::IAvoidStrategy {
public:
    rvc::Direction decideDirection(bool /*front*/, bool /*left*/, bool /*right*/) override {
        return rvc::Direction::FORWARD;
    }

    bool needsReverse(bool /*front*/, bool /*left*/, bool /*right*/) override {
        return false;
    }
};

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

class RVCControllerTest : public ::testing::Test {
protected:
    FakeMotor motor;
    FakeCleaner cleaner;
    FakeObstacleSensor obstacleSensor;
    FakeDustSensor dustSensor;
    FakeAvoidStrategy strategy;

    rvc::MovementManager movementMgr{motor, strategy};
    rvc::CleaningManager cleaningMgr{cleaner, [] {
                                         return std::int64_t{0};
                                     }};
    rvc::ObstacleSensorSubject obstacleSub{obstacleSensor};
    rvc::DustSensorSubject dustSub{dustSensor};

    rvc::RVCController controller{obstacleSensor, dustSensor,  motor,       cleaner,
                                  movementMgr,    cleaningMgr, obstacleSub, dustSub};

    static void suppressCout(const std::function<void()>& func) {
        std::ostringstream sink;
        std::streambuf* old = std::cout.rdbuf(sink.rdbuf());
        func();
        std::cout.rdbuf(old);
    }
};

// setState 호출 시 이전 상태의 onExit와 새 상태의 onEnter가 호출되는지 확인
TEST_F(RVCControllerTest, SetStateCallsEnterAndExit) {
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
TEST_F(RVCControllerTest, PowerOffDelegatesToState) {
    MockState state;
    controller.setState(&state);

    controller.powerOff();
    EXPECT_TRUE(state.powerOffCalled);
    EXPECT_EQ(state.contextPassed, &controller);
}

// 장애물 감지 이벤트가 현재 상태의 handleObstacle로 위임되는지 확인
TEST_F(RVCControllerTest, ObstacleDetectedDelegatesToState) {
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
TEST_F(RVCControllerTest, DustDetectedDelegatesToState) {
    MockState state;
    controller.setState(&state);

    controller.onDustDetected(true);
    EXPECT_TRUE(state.dustCalled);
    EXPECT_EQ(state.contextPassed, &controller);
    EXPECT_TRUE(state.lastDust);
}

// 생성 직후 매니저/Subject 가 모두 주입되어 있어야 한다
TEST_F(RVCControllerTest, ManagersAreInjectedOnConstruction) {
    EXPECT_EQ(controller.getMovementManager(), &movementMgr);
    EXPECT_EQ(controller.getCleaningManager(), &cleaningMgr);
    EXPECT_EQ(controller.getObstacleSensorSubject(), &obstacleSub);
    EXPECT_EQ(controller.getDustSensorSubject(), &dustSub);
    EXPECT_EQ(controller.getObstacleSensor(), &obstacleSensor);
}

// 생성 직후 currentState 는 OffState 로 초기화되어야 한다
TEST_F(RVCControllerTest, InitialStateIsOff) {
    auto* current = controller.getCurrentState();
    ASSERT_NE(current, nullptr);
    EXPECT_NE(dynamic_cast<rvc::OffState*>(current), nullptr);
}

// powerOn 호출 시 4개 디바이스가 모두 초기화에 성공하면 CleaningState 로 전이된다
TEST_F(RVCControllerTest, PowerOnTransitionsToCleaningStateOnSuccess) {
    suppressCout([&] {
        controller.powerOn();
    });

    auto* current = controller.getCurrentState();
    ASSERT_NE(current, nullptr);
    EXPECT_NE(dynamic_cast<rvc::CleaningState*>(current), nullptr);
}

// powerOn 성공 후 obstacle/dust Subject 에 controller 가 attach 되어 이벤트 흐름이 연결된다
TEST_F(RVCControllerTest, PowerOnAttachesObserverOnSuccess) {
    suppressCout([&] {
        controller.powerOn();
    });

    EXPECT_NO_THROW(obstacleSub.notify());
    EXPECT_NO_THROW(dustSub.notify());
    EXPECT_NO_THROW(suppressCout([&] {
        controller.powerOff();
    }));
}

// powerOff 호출 시 현재 state 의 handlePowerOff 가 위임되어 OffState 로 전이된다
TEST_F(RVCControllerTest, PowerOffTransitionsToOffState) {
    suppressCout([&] {
        controller.powerOn();
        controller.powerOff();
    });

    auto* current = controller.getCurrentState();
    ASSERT_NE(current, nullptr);
    EXPECT_NE(dynamic_cast<rvc::OffState*>(current), nullptr);
}

// tick 은 각 Subject 의 poll 과 CleaningManager::update 를 호출한다 (no-throw 검증)
TEST_F(RVCControllerTest, TickPollsSubjectsAndUpdatesManager) {
    suppressCout([&] {
        controller.powerOn();
    });
    EXPECT_NO_THROW(controller.tick());
}
