#include "rvc/MovementManager.hpp"
#include "rvc/i_avoid_strategy.hpp"
#include "rvc/i_motor.hpp"
#include "rvc/i_obstacle_sensor.hpp"

#include <gtest/gtest.h>

using namespace rvc;

class MockMotor : public IMotor {
public:
    Direction lastDirection = Direction::STOP;

    bool initialize() override {
        return true;
    }

    void move(Direction direction) override {
        lastDirection = direction;
    }
};

class MockStrategy : public IAvoidStrategy {
public:
    bool shouldReverse = false;
    Direction nextDir = Direction::FORWARD;

    Direction decideDirection(bool front, bool left, bool right) override {
        return nextDir;
    }

    bool needsReverse(bool f, bool l, bool r) override {
        return shouldReverse;
    }
};

class MockSensor : public IObstacleSensor {
public:
    bool front = false, left = false, right = false;

    bool initialize() override {
        return true;
    }

    bool isFrontDetected() override {
        return front;
    }

    bool isLeftDetected() override {
        return left;
    }

    bool isRightDetected() override {
        return right;
    }
};

class MovementManagerTest : public ::testing::Test {
protected:
    MockMotor* motor;
    MockStrategy* strategy;
    MockSensor* sensor;
    MovementManager* manager;

    // cppcheck-suppress unusedFunction
    void SetUp() override {
        motor = new MockMotor();
        strategy = new MockStrategy();
        sensor = new MockSensor();
        manager = new MovementManager(motor, strategy, sensor);
    }

    // cppcheck-suppress unusedFunction
    void TearDown() override {
        delete manager;
        delete sensor;
        delete strategy;
        delete motor;
    }
};

TEST_F(MovementManagerTest, MoveForward) {
    manager->moveForward();
    EXPECT_EQ(motor->lastDirection, Direction::FORWARD);
}

TEST_F(MovementManagerTest, MoveBackward) {
    manager->moveBackward();
    EXPECT_EQ(motor->lastDirection, Direction::BACKWARD);
}

TEST_F(MovementManagerTest, TurnLeft) {
    manager->turn(Direction::LEFT);
    EXPECT_EQ(motor->lastDirection, Direction::LEFT);
}

TEST_F(MovementManagerTest, TurnRight) {
    manager->turn(Direction::RIGHT);
    EXPECT_EQ(motor->lastDirection, Direction::RIGHT);
}

TEST_F(MovementManagerTest, Stop) {
    manager->stop();
    EXPECT_EQ(motor->lastDirection, Direction::STOP);
}

TEST_F(MovementManagerTest, AvoidanceWithoutReverseTurnLeft) {
    strategy->shouldReverse = false;
    strategy->nextDir = Direction::LEFT;
    manager->executeAvoidance(true, false, false);
    EXPECT_EQ(motor->lastDirection, Direction::LEFT);
}

TEST_F(MovementManagerTest, AvoidanceWithoutReverseTurnRight) {
    strategy->shouldReverse = false;
    strategy->nextDir = Direction::RIGHT;
    manager->executeAvoidance(true, true, false);
    EXPECT_EQ(motor->lastDirection, Direction::RIGHT);
}

// TEST_F(MovementManagerTest, EscapeLooptest) {
//     strategy->shouldReverse = true;
//     sensor->front = true;
//     sensor->left = true;
//     sensor->right = true;
//     manager->executeAvoidance(true, true, true);
//     EXPECT_EQ(motor->lastDirection, Direction::BACKWARD);
//     sensor->left = false;
//     EXPECT_EQ(motor->lastDirection, Direction::LEFT)
// }