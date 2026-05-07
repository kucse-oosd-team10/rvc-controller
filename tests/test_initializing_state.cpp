#include "rvc/cleaning_state.hpp"
#include "rvc/error_state.hpp"
#include "rvc/i_cleaner.hpp"
#include "rvc/i_dust_sensor.hpp"
#include "rvc/i_motor.hpp"
#include "rvc/i_obstacle_sensor.hpp"
#include "rvc/initializing_state.hpp"
#include "rvc/rvc_controller.hpp"
#include "rvc/types.hpp"

#include <sstream>
#include <vector>

#include <gtest/gtest.h>

namespace {

class FakeObstacleSensor : public rvc::IObstacleSensor {
public:
    bool initialize() override {
        initCallCount++;
        if (!initResults.empty()) {
            bool result = initResults.front();
            initResults.erase(initResults.begin());
            return result;
        }
        return defaultInitResult;
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

    int initCallCount{0};
    bool defaultInitResult{true};
    std::vector<bool> initResults;
};

class FakeDustSensor : public rvc::IDustSensor {
public:
    bool initialize() override {
        initCallCount++;
        return defaultInitResult;
    }

    bool isDustDetected() override {
        return false;
    } // NOLINT

    int initCallCount{0};
    bool defaultInitResult{true};
};

class FakeMotor : public rvc::IMotor {
public:
    bool initialize() override {
        initCallCount++;
        return defaultInitResult;
    }

    void move(rvc::Direction /*direction*/) override {
    }

    int initCallCount{0};
    bool defaultInitResult{true};
};

class FakeCleaner : public rvc::ICleaner {
public:
    bool initialize() override {
        initCallCount++;
        return defaultInitResult;
    }

    void setPower(rvc::PowerLevel /*level*/) override {
    }

    int initCallCount{0};
    bool defaultInitResult{true};
};

} // namespace

class InitializingStateTest : public ::testing::Test {
protected:
    FakeObstacleSensor obstacleSensor;
    FakeDustSensor dustSensor;
    FakeMotor motor;
    FakeCleaner cleaner;
    rvc::RVCController controller;

    // ErrorState::onEnter가 cout을 출력하므로 ErrorState 전이가 예상되는 테스트에서 억제한다
    static void suppressCout(const std::function<void()>& func) {
        std::ostringstream sink;
        std::streambuf* old = std::cout.rdbuf(sink.rdbuf());
        func();
        std::cout.rdbuf(old);
    }
};

// 4개 디바이스 모두 초기화 성공 → CleaningState로 전이
TEST_F(InitializingStateTest, AllDevicesSucceedTransitionsToCleaningState) {
    rvc::InitializingState state{obstacleSensor, dustSensor, motor, cleaner};
    state.onEnter(controller);

    ASSERT_NE(controller.getCurrentState(), nullptr);
    EXPECT_NE(dynamic_cast<rvc::CleaningState*>(controller.getCurrentState()), nullptr);
}

// 성공 시 각 디바이스의 initialize()가 정확히 1회씩 호출된다
TEST_F(InitializingStateTest, AllDevicesInitializedOnceOnSuccess) {
    rvc::InitializingState state{obstacleSensor, dustSensor, motor, cleaner};
    state.onEnter(controller);

    EXPECT_EQ(obstacleSensor.initCallCount, 1);
    EXPECT_EQ(dustSensor.initCallCount, 1);
    EXPECT_EQ(motor.initCallCount, 1);
    EXPECT_EQ(cleaner.initCallCount, 1);
}

// 1회 실패 후 2회차 성공 → CleaningState로 전이
TEST_F(InitializingStateTest, FirstAttemptFailsThenSucceedsTransitionsToCleaningState) {
    obstacleSensor.initResults = {false, true};

    rvc::InitializingState state{obstacleSensor, dustSensor, motor, cleaner};
    state.onEnter(controller);

    ASSERT_NE(controller.getCurrentState(), nullptr);
    EXPECT_NE(dynamic_cast<rvc::CleaningState*>(controller.getCurrentState()), nullptr);
    EXPECT_EQ(obstacleSensor.initCallCount, 2);
}

// 2회 실패 후 3회차 성공 → CleaningState로 전이
TEST_F(InitializingStateTest, TwoFailuresThenSuccessTransitionsToCleaningState) {
    obstacleSensor.initResults = {false, false, true};

    rvc::InitializingState state{obstacleSensor, dustSensor, motor, cleaner};
    state.onEnter(controller);

    ASSERT_NE(controller.getCurrentState(), nullptr);
    EXPECT_NE(dynamic_cast<rvc::CleaningState*>(controller.getCurrentState()), nullptr);
    EXPECT_EQ(obstacleSensor.initCallCount, 3);
}

// 3회 모두 실패 → ErrorState로 전이
TEST_F(InitializingStateTest, AllAttemptsFailTransitionsToErrorState) {
    obstacleSensor.defaultInitResult = false;

    suppressCout([&] {
        rvc::InitializingState state{obstacleSensor, dustSensor, motor, cleaner};
        state.onEnter(controller);
    });

    ASSERT_NE(controller.getCurrentState(), nullptr);
    EXPECT_NE(dynamic_cast<rvc::ErrorState*>(controller.getCurrentState()), nullptr);
}

// 3회 모두 실패 시 정확히 MAX_RETRY(3)회 시도된다
TEST_F(InitializingStateTest, RetriesExactlyMaxRetryTimes) {
    obstacleSensor.defaultInitResult = false;

    suppressCout([&] {
        rvc::InitializingState state{obstacleSensor, dustSensor, motor, cleaner};
        state.onEnter(controller);
    });

    EXPECT_EQ(obstacleSensor.initCallCount, 3);
}

// handleObstacle은 no-op이어야 한다
TEST_F(InitializingStateTest, HandleObstacleIsNoop) {
    rvc::InitializingState state{obstacleSensor, dustSensor, motor, cleaner};
    EXPECT_NO_THROW(state.handleObstacle(controller, true, true, true));
    EXPECT_EQ(controller.getCurrentState(), nullptr);
}

// handleDust는 no-op이어야 한다
TEST_F(InitializingStateTest, HandleDustIsNoop) {
    rvc::InitializingState state{obstacleSensor, dustSensor, motor, cleaner};
    EXPECT_NO_THROW(state.handleDust(controller, true));
    EXPECT_EQ(controller.getCurrentState(), nullptr);
}

// handlePowerOff는 no-op이어야 한다
TEST_F(InitializingStateTest, HandlePowerOffIsNoop) {
    rvc::InitializingState state{obstacleSensor, dustSensor, motor, cleaner};
    EXPECT_NO_THROW(state.handlePowerOff(controller));
    EXPECT_EQ(controller.getCurrentState(), nullptr);
}

// onExit는 no-op이어야 한다
TEST_F(InitializingStateTest, OnExitIsNoop) {
    rvc::InitializingState state{obstacleSensor, dustSensor, motor, cleaner};
    EXPECT_NO_THROW(state.onExit(controller));
}
