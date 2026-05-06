#include "rvc/cleaning_manager.hpp"
#include "rvc/cleaning_state.hpp"
#include "rvc/i_avoid_strategy.hpp"
#include "rvc/i_cleaner.hpp"
#include "rvc/i_motor.hpp"
#include "rvc/movement_manager.hpp"
#include "rvc/off_state.hpp"
#include "rvc/rvc_controller.hpp"
#include "rvc/types.hpp"

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

class FakeAvoidStrategy : public rvc::IAvoidStrategy {
public:
    rvc::Direction decideDirection(bool /*front*/, bool /*left*/, bool /*right*/) override {
        return rvc::Direction::LEFT;
    }

    bool needsReverse(bool /*front*/, bool /*left*/, bool /*right*/) override {
        return false;
    }
};

class FakeClock {
public:
    std::int64_t now() const {
        return currentTime_;
    }

    std::int64_t currentTime_{0};
};

class CleaningStateTest : public ::testing::Test {
protected:
    FakeMotor motor;
    FakeCleaner cleaner;
    FakeAvoidStrategy strategy;
    FakeClock clock;

    rvc::MovementManager movementMgr{motor, strategy};
    rvc::CleaningManager cleaningMgr{cleaner, [this] {
                                         return clock.now();
                                     }};

    rvc::RVCController controller;
    rvc::CleaningState state;

    void SetUp() override {
        controller.setMovementManager(&movementMgr);
        controller.setCleaningManager(&cleaningMgr);
    }
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

// 매니저가 nullptr인 상태에서 onEnter 호출해도 크래시가 없어야 한다.
TEST_F(CleaningStateTest, OnEnterHandlesNullManagers) {
    rvc::RVCController emptyController;
    EXPECT_NO_THROW(state.onEnter(emptyController));
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

// handleObstacle 호출 시 청소가 중지되어야 한다.
TEST_F(CleaningStateTest, HandleObstacleStopsCleaning) {
    state.onEnter(controller);
    cleaner.powers.clear();

    state.handleObstacle(controller, true, false, false);

    ASSERT_FALSE(cleaner.powers.empty());
    EXPECT_EQ(cleaner.powers.back(), rvc::PowerLevel::OFF);
    EXPECT_EQ(cleaningMgr.getPowerLevel(), rvc::PowerLevel::OFF);
}

// handleObstacle 호출 시 모터가 정지되어야 한다.
TEST_F(CleaningStateTest, HandleObstacleStopsMotor) {
    state.onEnter(controller);
    motor.moves.clear();

    state.handleObstacle(controller, true, false, false);

    ASSERT_FALSE(motor.moves.empty());
    EXPECT_EQ(motor.moves.back(), rvc::Direction::STOP);
}

// handleObstacle 호출 시 청소 중지 → 모터 정지 → 상태 전환 순서로 동작해야 한다.
TEST_F(CleaningStateTest, HandleObstacleOrdersStopBeforeStateTransition) {
    controller.setState(&state);
    motor.moves.clear();
    cleaner.powers.clear();

    state.handleObstacle(controller, true, true, true);

    EXPECT_FALSE(cleaner.powers.empty());
    EXPECT_EQ(cleaner.powers.back(), rvc::PowerLevel::OFF);
    EXPECT_FALSE(motor.moves.empty());
    EXPECT_EQ(motor.moves.back(), rvc::Direction::STOP);
}

// handleObstacle 호출 시 현재 state 의 onExit 이 트리거되어 상태 전환이 일어나야 한다.
TEST_F(CleaningStateTest, HandleObstacleTriggersStateTransition) {
    class SpyCleaningState : public rvc::CleaningState {
    public:
        void onExit(rvc::RVCController& ctx) override {
            rvc::CleaningState::onExit(ctx);
            onExitCalled = true;
        }

        bool onExitCalled{false};
    };

    SpyCleaningState spy;
    controller.setState(&spy);

    spy.handleObstacle(controller, true, false, false);

    EXPECT_TRUE(spy.onExitCalled);
}

// 매니저가 nullptr 일 때 handleObstacle 호출이 크래시 없이 동작해야 한다.
TEST_F(CleaningStateTest, HandleObstacleHandlesNullManagers) {
    rvc::RVCController emptyController;
    EXPECT_NO_THROW(state.handleObstacle(emptyController, true, false, false));
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
    class SpyCleaningState : public rvc::CleaningState {
    public:
        void onExit(rvc::RVCController& ctx) override {
            rvc::CleaningState::onExit(ctx);
            onExitCalled = true;
        }

        bool onExitCalled{false};
    };

    SpyCleaningState spy;
    controller.setState(&spy);

    spy.handleDust(controller, true);

    EXPECT_FALSE(spy.onExitCalled);
}

// 매니저가 nullptr 일 때 handleDust 호출이 크래시 없이 동작해야 한다.
TEST_F(CleaningStateTest, HandleDustHandlesNullManagers) {
    rvc::RVCController emptyController;
    EXPECT_NO_THROW(state.handleDust(emptyController, true));
    EXPECT_NO_THROW(state.handleDust(emptyController, false));
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
    class ProbeState : public rvc::CleaningState {
    public:
        rvc::IRVCState* lastNextState{nullptr};
    };

    ProbeState spy;
    controller.setState(&spy);

    spy.handlePowerOff(controller);

    // 전환 이후 onObstacleDetected 호출은 OffState 의 no-op 으로 흡수되어야 함
    motor.moves.clear();
    cleaner.powers.clear();
    controller.onObstacleDetected(true, true, true);

    EXPECT_TRUE(motor.moves.empty());
    EXPECT_TRUE(cleaner.powers.empty());
}

// handlePowerOff 호출 시 현재 state 의 onExit 이 트리거되어 상태 전환이 일어나야 한다.
TEST_F(CleaningStateTest, HandlePowerOffTriggersStateTransition) {
    class SpyCleaningState : public rvc::CleaningState {
    public:
        void onExit(rvc::RVCController& ctx) override {
            rvc::CleaningState::onExit(ctx);
            onExitCalled = true;
        }

        bool onExitCalled{false};
    };

    SpyCleaningState spy;
    controller.setState(&spy);

    spy.handlePowerOff(controller);

    EXPECT_TRUE(spy.onExitCalled);
}

// 매니저가 nullptr 일 때 handlePowerOff 호출이 크래시 없이 동작해야 한다.
TEST_F(CleaningStateTest, HandlePowerOffHandlesNullManagers) {
    rvc::RVCController emptyController;
    EXPECT_NO_THROW(state.handlePowerOff(emptyController));
}

// CleaningState onEnter → handleDust → handleObstacle 흐름이 일관되게 동작해야 한다.
TEST_F(CleaningStateTest, EndToEndScenarioBeforeAvoidance) {
    state.onEnter(controller);
    EXPECT_EQ(cleaningMgr.getPowerLevel(), rvc::PowerLevel::NORMAL);

    state.handleDust(controller, true);
    EXPECT_EQ(cleaningMgr.getPowerLevel(), rvc::PowerLevel::POWER_UP);

    state.handleObstacle(controller, true, false, false);
    EXPECT_EQ(cleaningMgr.getPowerLevel(), rvc::PowerLevel::OFF);
    EXPECT_EQ(motor.moves.back(), rvc::Direction::STOP);
}

} // namespace
