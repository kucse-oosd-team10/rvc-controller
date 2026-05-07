#include "rvc/cleaning_manager.hpp"
#include "rvc/cleaning_state.hpp"
#include "rvc/dust_sensor_subject.hpp"
#include "rvc/i_avoid_strategy.hpp"
#include "rvc/i_cleaner.hpp"
#include "rvc/i_dust_sensor.hpp"
#include "rvc/i_motor.hpp"
#include "rvc/i_obstacle_sensor.hpp"
#include "rvc/movement_manager.hpp"
#include "rvc/obstacle_sensor_subject.hpp"
#include "rvc/rvc_controller.hpp"
#include "rvc/types.hpp"

#include <algorithm>
#include <cstdint>
#include <vector>

#include <gtest/gtest.h>

namespace {

class FakeMotor : public rvc::IMotor {
public:
    bool initialize() override {
        return true;
    }

    void move(rvc::Direction direction) override {
        moves.push_back(direction);
    }

    std::vector<rvc::Direction> moves;
};

class FakeCleaner : public rvc::ICleaner {
public:
    bool initialize() override {
        return true;
    }

    void setPower(rvc::PowerLevel level) override {
        powers.push_back(level);
    }

    std::vector<rvc::PowerLevel> powers;
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
    rvc::Direction decideDirection(bool front, bool left, bool right) override {
        decideCalled = true;
        lastFront = front;
        lastLeft = left;
        lastRight = right;
        return rvc::Direction::LEFT;
    }

    bool needsReverse(bool front, bool left, bool right) override {
        needsReverseCalled = true;
        reverseFront = front;
        reverseLeft = left;
        reverseRight = right;
        return needsReverseValue;
    }

    bool needsReverseValue{false};
    bool needsReverseCalled{false};
    bool decideCalled{false};
    bool reverseFront{false};
    bool reverseLeft{false};
    bool reverseRight{false};
    bool lastFront{false};
    bool lastLeft{false};
    bool lastRight{false};
};

bool containsDirection(const std::vector<rvc::Direction>& moves, rvc::Direction target) {
    return std::find(moves.begin(), moves.end(), target) != moves.end();
}

class FakeClock {
public:
    [[nodiscard]] std::int64_t now() const {
        return currentTime_;
    }

    std::int64_t currentTime_{0};
};

class SpyCleaningState : public rvc::CleaningState {
public:
    void onExit(rvc::RVCController& ctx) override {
        rvc::CleaningState::onExit(ctx);
        onExitCalled = true;
    }

    bool onExitCalled{false};
};

} // namespace

class CleaningStateTest : public ::testing::Test {
protected:
    FakeMotor motor;
    FakeCleaner cleaner;
    FakeObstacleSensor obstacleSensor;
    FakeDustSensor dustSensor;
    FakeAvoidStrategy strategy;
    FakeClock clock;

    rvc::MovementManager movementMgr{motor, strategy};
    rvc::CleaningManager cleaningMgr{cleaner, [this] {
                                         return clock.now();
                                     }};
    rvc::ObstacleSensorSubject obstacleSub{obstacleSensor};
    rvc::DustSensorSubject dustSub{dustSensor};

    rvc::RVCController controller{obstacleSensor, dustSensor, motor, cleaner,
                                  movementMgr,    cleaningMgr, obstacleSub, dustSub};
    rvc::CleaningState state;
};

// onEnter 호출 시 모터가 전진 명령을 받아야 한다.
TEST_F(CleaningStateTest, OnEnterMovesForward) {
    state.onEnter(controller);

    ASSERT_FALSE(motor.moves.empty());
    EXPECT_EQ(motor.moves.front(), rvc::Direction::FORWARD);
}

// onEnter 호출 시 클리너가 NORMAL 파워로 청소를 시작해야 한다.
TEST_F(CleaningStateTest, OnEnterStartsCleaning) {
    state.onEnter(controller);

    EXPECT_EQ(cleaningMgr.getPowerLevel(), rvc::PowerLevel::NORMAL);
    ASSERT_FALSE(cleaner.powers.empty());
    EXPECT_EQ(cleaner.powers.back(), rvc::PowerLevel::NORMAL);
}

// onExit 호출이 어떠한 부수효과도 발생시키지 않아야 한다.
TEST_F(CleaningStateTest, OnExitIsNoOp) {
    state.onEnter(controller);
    motor.moves.clear();
    cleaner.powers.clear();

    state.onExit(controller);

    EXPECT_TRUE(motor.moves.empty());
    EXPECT_TRUE(cleaner.powers.empty());
}

// handleObstacle 호출 시 AvoidingState 인계 직전에 cleaner OFF 명령이 push 되어야 한다.
TEST_F(CleaningStateTest, HandleObstacleStopsCleaning) {
    state.onEnter(controller);
    cleaner.powers.clear();

    state.handleObstacle(controller, true, false, false);

    ASSERT_FALSE(cleaner.powers.empty());
    EXPECT_EQ(cleaner.powers.front(), rvc::PowerLevel::OFF);
}

// handleObstacle 호출 시 AvoidingState 진입 전에 모터 STOP 명령이 먼저 들어가야 한다.
TEST_F(CleaningStateTest, HandleObstacleStopsMotor) {
    state.onEnter(controller);
    motor.moves.clear();

    state.handleObstacle(controller, true, false, false);

    ASSERT_FALSE(motor.moves.empty());
    EXPECT_EQ(motor.moves.front(), rvc::Direction::STOP);
}

// handleObstacle 호출 시 청소 OFF → 모터 STOP → AvoidingState 전환 순서로 부수효과가 누적되어야 한다.
TEST_F(CleaningStateTest, HandleObstacleOrdersStopBeforeStateTransition) {
    controller.setState(&state);
    motor.moves.clear();
    cleaner.powers.clear();

    state.handleObstacle(controller, true, true, true);

    ASSERT_FALSE(cleaner.powers.empty());
    EXPECT_EQ(cleaner.powers.front(), rvc::PowerLevel::OFF);
    ASSERT_FALSE(motor.moves.empty());
    EXPECT_EQ(motor.moves.front(), rvc::Direction::STOP);
}

// handleObstacle 호출 시 현재 state 의 onExit 이 트리거되어 상태 전환이 일어나야 한다.
TEST_F(CleaningStateTest, HandleObstacleTriggersStateTransition) {
    SpyCleaningState spy;
    controller.setState(&spy);

    spy.handleObstacle(controller, true, false, false);

    EXPECT_TRUE(spy.onExitCalled);
}

// handleObstacle 의 front/left/right 값이 AvoidingState 회피 판단까지 전달되어야 한다.
TEST_F(CleaningStateTest, HandleObstaclePassesObstacleValuesToAvoidanceFlow) {
    controller.setState(&state);
    motor.moves.clear();
    cleaner.powers.clear();

    state.handleObstacle(controller, true, false, true);

    EXPECT_TRUE(strategy.needsReverseCalled);
    EXPECT_TRUE(strategy.decideCalled);
    EXPECT_TRUE(strategy.reverseFront);
    EXPECT_FALSE(strategy.reverseLeft);
    EXPECT_TRUE(strategy.reverseRight);
    EXPECT_TRUE(strategy.lastFront);
    EXPECT_FALSE(strategy.lastLeft);
    EXPECT_TRUE(strategy.lastRight);
    EXPECT_TRUE(containsDirection(motor.moves, rvc::Direction::STOP));
    EXPECT_TRUE(containsDirection(motor.moves, rvc::Direction::LEFT));

    auto* current = controller.getCurrentState();
    ASSERT_NE(current, nullptr);
    EXPECT_NE(dynamic_cast<rvc::CleaningState*>(current), nullptr);
}

// handleDust(true) 호출 시 청소 매니저로 dust 이벤트가 전달되어야 한다.
TEST_F(CleaningStateTest, HandleDustTrueForwardsToCleaningManager) {
    state.onEnter(controller);

    state.handleDust(controller, true);

    EXPECT_TRUE(cleaningMgr.getLatestDustDetected());
    EXPECT_EQ(cleaningMgr.getPowerLevel(), rvc::PowerLevel::POWER_UP);
}

// handleDust(false) 호출 시 청소 매니저의 dust 캐시가 false 로 갱신되어야 한다.
TEST_F(CleaningStateTest, HandleDustFalseUpdatesCache) {
    state.onEnter(controller);
    state.handleDust(controller, true);

    state.handleDust(controller, false);

    EXPECT_FALSE(cleaningMgr.getLatestDustDetected());
}

// handleDust 호출이 상태 전환을 발생시키지 않아야 한다.
TEST_F(CleaningStateTest, HandleDustDoesNotTransitionState) {
    SpyCleaningState spy;
    controller.setState(&spy);

    spy.handleDust(controller, true);

    EXPECT_FALSE(spy.onExitCalled);
}

// handlePowerOff 호출 시 청소가 중지되어야 한다.
TEST_F(CleaningStateTest, HandlePowerOffStopsCleaning) {
    state.onEnter(controller);
    cleaner.powers.clear();

    state.handlePowerOff(controller);

    ASSERT_FALSE(cleaner.powers.empty());
    EXPECT_EQ(cleaner.powers.back(), rvc::PowerLevel::OFF);
    EXPECT_EQ(cleaningMgr.getPowerLevel(), rvc::PowerLevel::OFF);
}

// handlePowerOff 호출 시 모터가 정지되어야 한다.
TEST_F(CleaningStateTest, HandlePowerOffStopsMotor) {
    state.onEnter(controller);
    motor.moves.clear();

    state.handlePowerOff(controller);

    ASSERT_FALSE(motor.moves.empty());
    EXPECT_EQ(motor.moves.back(), rvc::Direction::STOP);
}

// handlePowerOff 호출 후 controller 의 현재 state 가 OffState 로 전환되어야 한다.
TEST_F(CleaningStateTest, HandlePowerOffTransitionsToOffState) {
    SpyCleaningState spy;
    controller.setState(&spy);

    spy.handlePowerOff(controller);

    motor.moves.clear();
    cleaner.powers.clear();
    controller.onObstacleDetected(true, true, true);

    EXPECT_TRUE(motor.moves.empty());
    EXPECT_TRUE(cleaner.powers.empty());
}

// handlePowerOff 호출 시 현재 state 의 onExit 이 트리거되어 상태 전환이 일어나야 한다.
TEST_F(CleaningStateTest, HandlePowerOffTriggersStateTransition) {
    SpyCleaningState spy;
    controller.setState(&spy);

    spy.handlePowerOff(controller);

    EXPECT_TRUE(spy.onExitCalled);
}

// CleaningState onEnter → handleDust → handleObstacle → AvoidingState 인계 흐름이 일관되게 동작.
TEST_F(CleaningStateTest, EndToEndScenarioHandsOffToAvoiding) {
    state.onEnter(controller);
    EXPECT_EQ(cleaningMgr.getPowerLevel(), rvc::PowerLevel::NORMAL);

    state.handleDust(controller, true);
    EXPECT_EQ(cleaningMgr.getPowerLevel(), rvc::PowerLevel::POWER_UP);

    state.handleObstacle(controller, true, false, false);
    EXPECT_TRUE(std::find(cleaner.powers.begin(), cleaner.powers.end(), rvc::PowerLevel::OFF) !=
                cleaner.powers.end());
    EXPECT_TRUE(containsDirection(motor.moves, rvc::Direction::STOP));
    EXPECT_TRUE(containsDirection(motor.moves, rvc::Direction::LEFT));
}
