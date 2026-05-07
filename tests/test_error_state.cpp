#include "rvc/cleaning_manager.hpp"
#include "rvc/error_state.hpp"
#include "rvc/i_avoid_strategy.hpp"
#include "rvc/i_cleaner.hpp"
#include "rvc/i_motor.hpp"
#include "rvc/movement_manager.hpp"
#include "rvc/off_state.hpp"
#include "rvc/rvc_controller.hpp"
#include "rvc/types.hpp"

#include <sstream>
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
        return rvc::Direction::FORWARD;
    }

    bool needsReverse(bool /*front*/, bool /*left*/, bool /*right*/) override {
        return false;
    }
};

class FakeClock {
public:
    static std::int64_t now() {
        return 0;
    }
};

} // namespace

class ErrorStateTest : public ::testing::Test {
protected:
    FakeMotor motor;
    FakeCleaner cleaner;
    FakeAvoidStrategy strategy;
    rvc::MovementManager movementMgr{motor, strategy};
    rvc::CleaningManager cleaningMgr{cleaner, [] {
                                         return FakeClock::now();
                                     }};
    rvc::RVCController controller;
    rvc::ErrorState state;

    ErrorStateTest() {
        controller.setMovementManager(&movementMgr);
        controller.setCleaningManager(&cleaningMgr);
    }
};

// onEnter 호출 시 에러 메시지가 출력되어야 한다
TEST_F(ErrorStateTest, OnEnterPrintsErrorMessage) {
    std::ostringstream captured;
    std::streambuf* old = std::cout.rdbuf(captured.rdbuf());
    state.onEnter(controller);
    std::cout.rdbuf(old);

    EXPECT_NE(captured.str().find("Error"), std::string::npos);
}

// onEnter 호출 시 모터가 정지되어야 한다
TEST_F(ErrorStateTest, OnEnterStopsMotor) {
    movementMgr.moveForward();
    motor.moves.clear();

    std::ostringstream sink;
    std::streambuf* old = std::cout.rdbuf(sink.rdbuf());
    state.onEnter(controller);
    std::cout.rdbuf(old);

    ASSERT_FALSE(motor.moves.empty());
    EXPECT_EQ(motor.moves.back(), rvc::Direction::STOP);
}

// onEnter 호출 시 클리너가 OFF되어야 한다
TEST_F(ErrorStateTest, OnEnterStopsCleaning) {
    cleaningMgr.startCleaning();
    cleaner.powers.clear();

    std::ostringstream sink;
    std::streambuf* old = std::cout.rdbuf(sink.rdbuf());
    state.onEnter(controller);
    std::cout.rdbuf(old);

    ASSERT_FALSE(cleaner.powers.empty());
    EXPECT_EQ(cleaner.powers.back(), rvc::PowerLevel::OFF);
}

// 매니저가 nullptr일 때도 onEnter가 크래시 없이 동작해야 한다
TEST_F(ErrorStateTest, OnEnterIsRobustToNullManagers) {
    rvc::RVCController empty;
    std::ostringstream sink;
    std::streambuf* old = std::cout.rdbuf(sink.rdbuf());
    EXPECT_NO_THROW(state.onEnter(empty));
    std::cout.rdbuf(old);
}

// handleObstacle은 no-op이어야 한다
TEST_F(ErrorStateTest, HandleObstacleIsNoop) {
    state.handleObstacle(controller, true, true, true);
    EXPECT_TRUE(motor.moves.empty());
}

// handleDust는 no-op이어야 한다
TEST_F(ErrorStateTest, HandleDustIsNoop) {
    state.handleDust(controller, true);
    EXPECT_EQ(cleaningMgr.getPowerLevel(), rvc::PowerLevel::OFF);
}

// handlePowerOff 호출 시 OffState로 전이되어야 한다
TEST_F(ErrorStateTest, HandlePowerOffTransitionsToOffState) {
    state.handlePowerOff(controller);

    ASSERT_NE(controller.getCurrentState(), nullptr);
    EXPECT_NE(dynamic_cast<rvc::OffState*>(controller.getCurrentState()), nullptr);
}

// onExit는 no-op이어야 한다
TEST_F(ErrorStateTest, OnExitIsNoop) {
    EXPECT_NO_THROW(state.onExit(controller));
}
