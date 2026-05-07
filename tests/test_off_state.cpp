#include "rvc/cleaning_manager.hpp"
#include "rvc/dust_sensor_subject.hpp"
#include "rvc/i_avoid_strategy.hpp"
#include "rvc/i_cleaner.hpp"
#include "rvc/i_dust_sensor.hpp"
#include "rvc/i_motor.hpp"
#include "rvc/i_obstacle_sensor.hpp"
#include "rvc/movement_manager.hpp"
#include "rvc/obstacle_sensor_subject.hpp"
#include "rvc/off_state.hpp"
#include "rvc/rvc_controller.hpp"
#include "rvc/types.hpp"

#include <cstdint>

#include <gtest/gtest.h>

using namespace rvc;

namespace {

class FakeMotor : public IMotor {
public:
    bool initialize() override {
        return true;
    }

    void move(Direction /*direction*/) override {
    }
};

class FakeCleaner : public ICleaner {
public:
    bool initialize() override {
        return true;
    }

    void setPower(PowerLevel /*level*/) override {
    }
};

class FakeObstacleSensor : public IObstacleSensor {
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

class FakeDustSensor : public IDustSensor {
public:
    bool initialize() override {
        return true;
    }

    bool isDustDetected() override {
        return false;
    }
};

class FakeAvoidStrategy : public IAvoidStrategy {
public:
    Direction decideDirection(bool /*front*/, bool /*left*/, bool /*right*/) override {
        return Direction::FORWARD;
    }

    bool needsReverse(bool /*front*/, bool /*left*/, bool /*right*/) override {
        return false;
    }
};

class SpyOffState : public OffState {
public:
    void onExit(RVCController& ctx) override {
        OffState::onExit(ctx);
        onExitCalled = true;
    }

    bool onExitCalled{false};
};

} // namespace

class OffStateTest : public ::testing::Test {
protected:
    FakeMotor motor;
    FakeCleaner cleaner;
    FakeObstacleSensor obstacleSensor;
    FakeDustSensor dustSensor;
    FakeAvoidStrategy strategy;

    MovementManager movementMgr{motor, strategy};
    CleaningManager cleaningMgr{cleaner, [] {
                                    return std::int64_t{0};
                                }};
    ObstacleSensorSubject obstacleSub{obstacleSensor};
    DustSensorSubject dustSub{dustSensor};

    RVCController controller{obstacleSensor, dustSensor,  motor,       cleaner,
                             movementMgr,    cleaningMgr, obstacleSub, dustSub};
    OffState state;
};

TEST_F(OffStateTest, onEnter) {
    EXPECT_NO_THROW(state.onEnter(controller));
}

TEST_F(OffStateTest, HandlePowerOffDoesNotTransition) {
    SpyOffState spyState;
    controller.setState(&spyState);
    ASSERT_FALSE(spyState.onExitCalled);

    spyState.handlePowerOff(controller);
    EXPECT_FALSE(spyState.onExitCalled);
}

TEST_F(OffStateTest, HandleObstacleDoesNotTransition) {
    SpyOffState spyState;
    controller.setState(&spyState);

    spyState.handleObstacle(controller, true, true, true);
    EXPECT_FALSE(spyState.onExitCalled);
}

TEST_F(OffStateTest, HandleDustDoesNotTransition) {
    SpyOffState spyState;
    controller.setState(&spyState);

    spyState.handleDust(controller, true);
    EXPECT_FALSE(spyState.onExitCalled);
}
