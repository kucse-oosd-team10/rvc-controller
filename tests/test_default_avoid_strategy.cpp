#include "rvc/default_avoid_strategy.hpp"

#include <gtest/gtest.h>

using namespace rvc;

TEST(AvoidStrategyTest, Allblock) {
    DefaultAvoidStrategy strategy;
    EXPECT_EQ(strategy.decideDirection(true, true, true), Direction::BACKWARD);
    EXPECT_TRUE(strategy.needsReverse(true, true, true));
}

TEST(AvoidStrategyTest, FrontAndLeftBlock) {
    DefaultAvoidStrategy strategy;
    EXPECT_EQ(strategy.decideDirection(true, true, false), Direction::RIGHT);
    EXPECT_FALSE(strategy.needsReverse(true, true, false));
}

TEST(AvoidStrategyTest, FrontAndRightBlock) {
    DefaultAvoidStrategy strategy;
    EXPECT_EQ(strategy.decideDirection(true, false, true), Direction::LEFT);
    EXPECT_FALSE(strategy.needsReverse(true, false, true));
}

TEST(AvoidStrategyTest, FrontBlock) {
    DefaultAvoidStrategy strategy;
    EXPECT_EQ(strategy.decideDirection(true, false, false), Direction::LEFT);
    EXPECT_FALSE(strategy.needsReverse(true, false, false));
}

TEST(AvoidStrategyTest, NoObstacle) {
    DefaultAvoidStrategy strategy;
    EXPECT_EQ(strategy.decideDirection(false, false, false), Direction::FORWARD);
    EXPECT_FALSE(strategy.needsReverse(false, false, false));
}

TEST(AvoidStrategyTest, OnlyLeftSideObstacle) {
    DefaultAvoidStrategy strategy;
    EXPECT_EQ(strategy.decideDirection(false, true, false), Direction::FORWARD);
    EXPECT_FALSE(strategy.needsReverse(false, true, false));
}

TEST(AvoidStrategyTest, OnlyRightSideObstacle) {
    DefaultAvoidStrategy strategy;
    EXPECT_EQ(strategy.decideDirection(false, false, true), Direction::FORWARD);
    EXPECT_FALSE(strategy.needsReverse(false, false, true));
}

TEST(AvoidStrategyTest, OnlySideObstacle) {
    DefaultAvoidStrategy strategy;
    EXPECT_EQ(strategy.decideDirection(false, true, true), Direction::FORWARD);
    EXPECT_FALSE(strategy.needsReverse(false, true, true));
}