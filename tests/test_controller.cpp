#include "rvc/controller.hpp"

#include <gtest/gtest.h>

using rvc::CleanMode;
using rvc::Controller;
using rvc::Direction;

class ControllerTest : public ::testing::Test {
protected:
    Controller ctrl;
};

TEST_F(ControllerTest, GoesForwardByDefault) {
    ctrl.tick();
    EXPECT_EQ(ctrl.getDirection(), Direction::Forward);
    EXPECT_EQ(ctrl.getCleanMode(), CleanMode::On);
}

TEST_F(ControllerTest, TurnsLeftOnFrontObstacle) {
    ctrl.setFrontSensor(true);
    ctrl.tick();
    EXPECT_EQ(ctrl.getDirection(), Direction::Left);
}

TEST_F(ControllerTest, TurnsRightWhenFrontAndLeftBlocked) {
    ctrl.setFrontSensor(true);
    ctrl.setLeftSensor(true);
    ctrl.tick();
    EXPECT_EQ(ctrl.getDirection(), Direction::Right);
}

TEST_F(ControllerTest, TurnsLeftWhenFrontAndRightBlocked) {
    ctrl.setFrontSensor(true);
    ctrl.setRightSensor(true);
    ctrl.tick();
    EXPECT_EQ(ctrl.getDirection(), Direction::Left);
}

TEST_F(ControllerTest, GoesBackwardWhenAllBlocked) {
    ctrl.setFrontSensor(true);
    ctrl.setLeftSensor(true);
    ctrl.setRightSensor(true);
    ctrl.tick();
    EXPECT_EQ(ctrl.getDirection(), Direction::Backward);
}

TEST_F(ControllerTest, PowerUpOnDust) {
    ctrl.setDustSensor(true);
    ctrl.tick();
    EXPECT_EQ(ctrl.getCleanMode(), CleanMode::PowerUp);
}

TEST_F(ControllerTest, NormalCleanWhenNoDust) {
    ctrl.setDustSensor(false);
    ctrl.tick();
    EXPECT_EQ(ctrl.getCleanMode(), CleanMode::On);
}

TEST_F(ControllerTest, StopsWhenAllBlocked) {
    ctrl.setFrontSensor(true);
    ctrl.setLeftSensor(true);
    ctrl.setRightSensor(true);
    ctrl.tick();
    EXPECT_TRUE(ctrl.isStopped());
}

TEST_F(ControllerTest, SensorsResetAfterTick) {
    ctrl.setFrontSensor(true);
    ctrl.tick();
    // Sensors reset, so second tick with no input should go forward
    ctrl.tick();
    EXPECT_EQ(ctrl.getDirection(), Direction::Forward);
    EXPECT_FALSE(ctrl.isStopped());
}