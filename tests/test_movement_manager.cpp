#include "rvc/i_avoid_strategy.hpp"
#include "rvc/i_motor.hpp"
#include "rvc/movement_manager.hpp"

#include <gtest/gtest.h>

using namespace rvc;

class TestMotor : public IMotor {
public:
    Direction lastDirection{Direction::STOP};
    int moveCount{0};

    bool initialize() override {
        return true;
    }

    void move(Direction direction) override {
        lastDirection = direction;
        moveCount++;
    }
};

class TestStrategy : public IAvoidStrategy {
public:
    Direction directionToReturn{Direction::STOP};

    Direction decideDirection(bool front, bool left, bool right) override {
        return directionToReturn;
    }

    bool needsReverse(bool front, bool left, bool right) override {
        return false;
    }
};

class MovementManagerTest : public ::testing::Test {
protected:
    TestMotor motor;
    TestStrategy strategy;
    MovementManager manager{motor, strategy};
};

TEST_F(MovementManagerTest, MoveForward) {
    manager.moveForward();
    EXPECT_EQ(motor.lastDirection, Direction::FORWARD);
    EXPECT_EQ(motor.moveCount, 1);
}

TEST_F(MovementManagerTest, MoveBackward) {
    manager.moveBackward();
    EXPECT_EQ(motor.lastDirection, Direction::BACKWARD);
    EXPECT_EQ(motor.moveCount, 1);
}

TEST_F(MovementManagerTest, TurnLeftAndRight) {
    manager.turn(Direction::LEFT);
    EXPECT_EQ(motor.lastDirection, Direction::LEFT);

    manager.turn(Direction::RIGHT);
    EXPECT_EQ(motor.lastDirection, Direction::RIGHT);
}

TEST_F(MovementManagerTest, StopAfterMoving) {
    manager.moveForward();
    int currentCount = motor.moveCount;

    manager.stop();
    EXPECT_EQ(motor.lastDirection, Direction::STOP);
    EXPECT_EQ(motor.moveCount, currentCount + 1);
}

TEST_F(MovementManagerTest, ShouldNotMoveMotorIfAlreadyInSameDirection) {
    manager.moveForward();
    manager.moveForward();

    EXPECT_EQ(motor.moveCount, 1);
}

TEST_F(MovementManagerTest, AvoidanceWithoutReverseTurnLeft) {
    strategy.directionToReturn = Direction::LEFT;

    manager.executeAvoidance(true, false, false);
    EXPECT_EQ(motor.lastDirection, Direction::LEFT);
}

TEST_F(MovementManagerTest, AvoidanceWithoutReverseTurnRight) {
    strategy.directionToReturn = Direction::RIGHT;

    manager.executeAvoidance(true, true, false);
    EXPECT_EQ(motor.lastDirection, Direction::RIGHT);
}

TEST_F(MovementManagerTest, EscapeLoopWithDirectionChange) {
    // 사방이 막힌 상황(true, true, true) 가정
    strategy.directionToReturn = Direction::LEFT;

    manager.executeAvoidance(true, true, true);
    EXPECT_EQ(motor.lastDirection, Direction::LEFT);
}

TEST_F(MovementManagerTest, testNeedsReverse) {
    strategy.directionToReturn = Direction::LEFT;
    bool result = manager.needsReverse(true, true, true);
    EXPECT_FALSE(result);
}