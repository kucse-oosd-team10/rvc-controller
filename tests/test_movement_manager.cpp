#include "rvc/MovementManager.hpp"
#include "rvc/i_avoid_strategy.hpp"
#include "rvc/i_motor.hpp"
#include "rvc/i_obstacle_sensor.hpp"

#include <gmock/gmock.h>

#include <gtest/gtest.h>

using namespace rvc;

class MockMotor : public IMotor {
public:
    MOCK_METHOD(bool, initialize, (), (override));
    MOCK_METHOD(void, move, (Direction direction), (override));
};

class MockStrategy : public IAvoidStrategy {
public:
    MOCK_METHOD(Direction, decideDirection, (bool front, bool left, bool right), (override));
    MOCK_METHOD(bool, needsReverse, (bool front, bool left, bool right), (override));
};

class MockSensor : public IObstacleSensor {
public:
    // MOCK_METHOD(반환타입, 함수이름, (인자), (한정자));
    MOCK_METHOD(bool, initialize, (), (override));
    MOCK_METHOD(bool, isFrontDetected, (), (override));
    MOCK_METHOD(bool, isLeftDetected, (), (override));
    MOCK_METHOD(bool, isRightDetected, (), (override));
};

class MovementManagerTest : public ::testing::Test {
protected:
    MockMotor motor;
    MockStrategy strategy;
    MockSensor sensor;
    MovementManager manager{motor, strategy};
};

TEST_F(MovementManagerTest, MoveForward) {
    EXPECT_CALL(motor, move(Direction::FORWARD)).Times(1);
    manager.moveForward();
}

TEST_F(MovementManagerTest, MoveBackward) {
    EXPECT_CALL(motor, move(Direction::BACKWARD)).Times(1);
    manager.moveBackward();
}

TEST_F(MovementManagerTest, TurnLeft) {
    EXPECT_CALL(motor, move(Direction::LEFT)).Times(1);
    manager.turn(Direction::LEFT);
}

TEST_F(MovementManagerTest, TurnRight) {
    EXPECT_CALL(motor, move(Direction::RIGHT)).Times(1);
    manager.turn(Direction::RIGHT);
}

TEST_F(MovementManagerTest, Stop) {
    manager.moveForward();
    EXPECT_CALL(motor, move(Direction::STOP)).Times(1);
    manager.stop();
}

TEST_F(MovementManagerTest, AvoidanceWithoutReverseTurnLeft) {
    EXPECT_CALL(strategy, needsReverse(true, false, false)).WillOnce(testing::Return(false));
    EXPECT_CALL(strategy, decideDirection(true, false, false))
        .WillOnce(testing::Return(Direction::LEFT));
    EXPECT_CALL(motor, move(Direction::LEFT)).Times(1);
    manager.executeAvoidance(true, false, false);
}

TEST_F(MovementManagerTest, AvoidanceWithoutReverseTurnRight) {
    EXPECT_CALL(strategy, needsReverse(true, true, false)).WillOnce(testing::Return(false));
    EXPECT_CALL(strategy, decideDirection(true, true, false))
        .WillOnce(testing::Return(Direction::RIGHT));
    EXPECT_CALL(motor, move(Direction::RIGHT)).Times(1);
    manager.executeAvoidance(true, true, false);
}

TEST_F(MovementManagerTest, EscapeLoopWithLeftTurn) {
    EXPECT_CALL(strategy, needsReverse(true, true, true)).WillOnce(testing::Return(true));
    EXPECT_CALL(strategy, decideDirection(true, true, true))
        .WillOnce(testing::Return(Direction::LEFT));
    EXPECT_CALL(motor, move(Direction::LEFT)).Times(1);
    manager.executeAvoidance(true, true, true);
}

TEST_F(MovementManagerTest, EscapeLoopWithRightTurn) {
    EXPECT_CALL(strategy, needsReverse(true, true, true)).WillOnce(testing::Return(true));
    EXPECT_CALL(strategy, decideDirection(true, true, true))
        .WillOnce(testing::Return(Direction::RIGHT));
    EXPECT_CALL(motor, move(Direction::RIGHT)).Times(1);
    manager.executeAvoidance(true, true, true);
}