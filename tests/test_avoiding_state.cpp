#include "rvc/avoiding_state.hpp"
#include "rvc/cleaning_manager.hpp"
#include "rvc/cleaning_state.hpp"
#include "rvc/default_avoid_strategy.hpp"
#include "rvc/dust_sensor_subject.hpp"
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
        directions.push_back(direction);
        last = direction;
    }

    rvc::Direction last{rvc::Direction::STOP};
    std::vector<rvc::Direction> directions;
};

class FakeCleaner : public rvc::ICleaner {
public:
    bool initialize() override {
        return true;
    }

    void setPower(rvc::PowerLevel level) override {
        last = level;
        callCount++;
    }

    rvc::PowerLevel last{rvc::PowerLevel::OFF};
    int callCount{0};
};

class FakeObstacleSensor : public rvc::IObstacleSensor {
public:
    bool initialize() override {
        return true;
    }

    bool isFrontDetected() override {
        front_reads++;
        if (!front_seq.empty()) {
            bool value = front_seq.front();
            front_seq.erase(front_seq.begin());
            return value;
        }
        return front_default;
    }

    bool isLeftDetected() override {
        left_reads++;
        if (!left_seq.empty()) {
            bool value = left_seq.front();
            left_seq.erase(left_seq.begin());
            return value;
        }
        return left_default;
    }

    bool isRightDetected() override {
        right_reads++;
        if (!right_seq.empty()) {
            bool value = right_seq.front();
            right_seq.erase(right_seq.begin());
            return value;
        }
        return right_default;
    }

    bool front_default{false};
    bool left_default{false};
    bool right_default{false};
    std::vector<bool> front_seq;
    std::vector<bool> left_seq;
    std::vector<bool> right_seq;
    int front_reads{0};
    int left_reads{0};
    int right_reads{0};
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

class MockClock {
public:
    [[nodiscard]] std::int64_t now() const {
        return currentTime;
    }

    std::int64_t currentTime{0};
};

} // namespace

class AvoidingStateTest : public ::testing::Test {
protected:
    FakeMotor motor;
    FakeCleaner cleaner;
    FakeObstacleSensor sensor;
    FakeDustSensor dustSensor;
    rvc::DefaultAvoidStrategy strategy;
    MockClock clock;

    rvc::MovementManager mm{motor, strategy};
    rvc::CleaningManager cm{cleaner, [this] {
                                return clock.now();
                            }};
    rvc::ObstacleSensorSubject obstacleSub{sensor};
    rvc::DustSensorSubject dustSub{dustSensor};

    rvc::RVCController controller{sensor, dustSensor, motor, cleaner, mm, cm, obstacleSub, dustSub};

    static bool containsDirection(const std::vector<rvc::Direction>& dirs, rvc::Direction target) {
        return std::find(dirs.begin(), dirs.end(), target) != dirs.end();
    }
};

// 전방만 막혔을 때 후진 없이 곧바로 좌측 회피로 turn 호출이 일어난다
TEST_F(AvoidingStateTest, OnEnterFrontOnlyTurnsLeftWithoutReversing) {
    rvc::AvoidingState state{true, false, false};
    state.onEnter(controller);

    EXPECT_FALSE(containsDirection(motor.directions, rvc::Direction::BACKWARD));
    EXPECT_TRUE(containsDirection(motor.directions, rvc::Direction::LEFT));
}

// 전방+좌측이 막혔을 때 후진 없이 우측으로 회피한다
TEST_F(AvoidingStateTest, OnEnterFrontLeftBlockedTurnsRight) {
    rvc::AvoidingState state{true, true, false};
    state.onEnter(controller);

    EXPECT_FALSE(containsDirection(motor.directions, rvc::Direction::BACKWARD));
    EXPECT_TRUE(containsDirection(motor.directions, rvc::Direction::RIGHT));
}

// 사방이 막힌 상황에서 한쪽이 풀릴 때까지 후진 후 회피 방향을 결정한다
TEST_F(AvoidingStateTest, OnEnterAllBlockedReversesUntilSideClears) {
    sensor.left_seq = {true, false, false};
    sensor.right_seq = {true, true, false};
    sensor.front_seq = {false};

    rvc::AvoidingState state{true, true, true};
    state.onEnter(controller);

    EXPECT_EQ(motor.directions.front(), rvc::Direction::BACKWARD);
    EXPECT_EQ(motor.last, rvc::Direction::FORWARD);
    EXPECT_GE(sensor.left_reads, 3);
    EXPECT_GE(sensor.right_reads, 3);
    EXPECT_GE(sensor.front_reads, 1);
}

// 후진 한 번에 좌측이 풀리면 후진 루프가 1회만 실행된다
TEST_F(AvoidingStateTest, OnEnterAllBlockedExitsAfterSingleReverseWhenLeftClears) {
    sensor.left_seq = {false, false};
    sensor.right_seq = {true, false};
    sensor.front_seq = {false};

    rvc::AvoidingState state{true, true, true};
    state.onEnter(controller);

    EXPECT_EQ(sensor.left_reads, 2);
    EXPECT_EQ(sensor.right_reads, 2);
}

// onEnter 회피 완료 후 currentState_ 가 CleaningState 인스턴스로 전환되어야 한다.
TEST_F(AvoidingStateTest, OnEnterTransitionsToCleaningStateAfterAvoidance) {
    rvc::AvoidingState state{true, false, false};
    controller.setState(&state);

    auto* current = controller.getCurrentState();
    ASSERT_NE(current, nullptr);
    EXPECT_NE(dynamic_cast<rvc::CleaningState*>(current), nullptr);
}

// 회피 중 handleDust(true) 는 CleaningManager 캐시를 true 로 갱신한다
TEST_F(AvoidingStateTest, HandleDustTrueUpdatesCleaningManagerCache) {
    rvc::AvoidingState state{false, false, false};
    state.handleDust(controller, true);

    EXPECT_TRUE(cm.getLatestDustDetected());
}

// 회피 중 handleDust(false) 는 CleaningManager 캐시를 false 로 갱신한다
TEST_F(AvoidingStateTest, HandleDustFalseUpdatesCleaningManagerCache) {
    cm.handleDustDetected(true);
    rvc::AvoidingState state{false, false, false};
    state.handleDust(controller, false);

    EXPECT_FALSE(cm.getLatestDustDetected());
}

// 청소가 OFF 상태인 회피 중에 handleDust(true) 를 받아도 cleaner 는 켜지지 않는다
TEST_F(AvoidingStateTest, HandleDustDuringAvoidanceDoesNotPowerOnCleaner) {
    rvc::AvoidingState state{false, false, false};
    state.handleDust(controller, true);

    EXPECT_EQ(cleaner.last, rvc::PowerLevel::OFF);
    EXPECT_EQ(cm.getPowerLevel(), rvc::PowerLevel::OFF);
}

// 회피 중에는 새로 들어온 handleObstacle 이벤트를 무시하므로 모터 호출이 발생하지 않는다
TEST_F(AvoidingStateTest, HandleObstacleIsIgnoredDuringAvoidance) {
    rvc::AvoidingState state{false, false, false};
    state.handleObstacle(controller, true, true, true);

    EXPECT_TRUE(motor.directions.empty());
    EXPECT_EQ(sensor.front_reads, 0);
}

// onExit 호출은 부작용 없이 안전하게 종료된다
TEST_F(AvoidingStateTest, OnExitHasNoSideEffect) {
    rvc::AvoidingState state{false, false, false};
    EXPECT_NO_THROW(state.onExit(controller));
    EXPECT_TRUE(motor.directions.empty());
}

// handlePowerOff 시 모터 정지, 청소 중지, OffState 로의 전이가 모두 발생한다
TEST_F(AvoidingStateTest, HandlePowerOffStopsAndTransitionsToOff) {
    cm.handleDustDetected(true);
    cm.startCleaning();
    motor.directions.clear();

    rvc::AvoidingState state{false, false, false};
    state.handlePowerOff(controller);

    EXPECT_EQ(motor.last, rvc::Direction::STOP);
    EXPECT_EQ(cm.getPowerLevel(), rvc::PowerLevel::OFF);
    EXPECT_NE(controller.getCurrentState(), nullptr);
    EXPECT_NE(dynamic_cast<rvc::OffState*>(controller.getCurrentState()), nullptr);
}
