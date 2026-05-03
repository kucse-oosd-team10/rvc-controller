#include "rvc/DefaultAvoidStrategy.hpp"

#include <gtest/gtest.h>

using namespace rvc;

class AvoidStrategyTest : public ::testing::Test {
protected:
    DefaultAvoidStrategy strategy;
};

TEST_F(AvoidStrategyTest, Allblock) {
    EXPECT_EQ(strategy.decideDirection(true, true, true), Direction::BACKWARD);
    EXPECT_TRUE(strategy.needsReverse(true, true, true));
}

TEST_F(AvoidStrategyTest, FrontAndLeftBlock) {
    EXPECT_EQ(strategy.decideDirection(true, true, false), Direction::RIGHT);
    EXPECT_FALSE(strategy.needsReverse(true, true, false));
}

TEST_F(AvoidStrategyTest, FrontAndRightBlock) {
    EXPECT_EQ(strategy.decideDirection(true, false, true), Direction::LEFT);
    EXPECT_FALSE(strategy.needsReverse(true, false, true));
}

TEST_F(AvoidStrategyTest, FrontBlock) {
    EXPECT_EQ(strategy.decideDirection(true, false, false), Direction::LEFT);
    EXPECT_FALSE(strategy.needsReverse(true, false, false));
}

TEST_F(AvoidStrategyTest, NoObstacle) {
    EXPECT_EQ(strategy.decideDirection(false, false, false), Direction::FORWARD);
    EXPECT_FALSE(strategy.needsReverse(false, false, false));
}

TEST_F(AvoidStrategyTest, OnlyLeftSideObstacle) {
    EXPECT_EQ(strategy.decideDirection(false, true, false), Direction::FORWARD);
    EXPECT_FALSE(strategy.needsReverse(false, true, false));
}

TEST_F(AvoidStrategyTest, OnlyRightSideObstacle) {
    EXPECT_EQ(strategy.decideDirection(false, false, true), Direction::FORWARD);
    EXPECT_FALSE(strategy.needsReverse(false, false, true));
}

TEST_F(AvoidStrategyTest, OnlySideObstacle) {
    EXPECT_EQ(strategy.decideDirection(false, true, true), Direction::FORWARD);
    EXPECT_FALSE(strategy.needsReverse(false, true, true));
}