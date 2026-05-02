#include "rvc/timer.hpp"

#include <cstdint>

#include <gtest/gtest.h>

using rvc::Timer;

namespace {
class TestClock {
public:
    std::int64_t now() const {
        return now_;
    }

    void increase(std::int64_t t) {
        now_ += t;
    }

private:
    std::int64_t now_{0};
};
} // namespace

class TimerTest : public ::testing::Test {
protected:
    TestClock clock;
    Timer timer{[this] {
        return clock.now();
    }};
};

TEST_F(TimerTest, NoRunInit) {
    EXPECT_FALSE(timer.isRunning());
    EXPECT_FALSE(timer.isExpired());
}

TEST_F(TimerTest, StartMakeRun) {
    timer.setDuration(100);
    timer.start();
    EXPECT_TRUE(timer.isRunning());
    EXPECT_FALSE(timer.isExpired());
}

TEST_F(TimerTest, StopIsStop) {
    timer.start();
    EXPECT_TRUE(timer.isRunning());

    timer.stop();
    EXPECT_FALSE(timer.isRunning());
}

TEST_F(TimerTest, CheckExpire) {
    timer.setDuration(100);
    timer.start();
    clock.increase(99);
    EXPECT_FALSE(timer.isExpired());
    clock.increase(1);
    EXPECT_TRUE(timer.isExpired());
}

TEST_F(TimerTest, StoppedTimerNotExpired) {
    timer.setDuration(100);
    timer.start();
    clock.increase(200);
    timer.stop();
    EXPECT_FALSE(timer.isExpired());
}

TEST_F(TimerTest, ResetRestartsCountdown) {
    timer.setDuration(100);
    timer.start();
    clock.increase(80);
    timer.reset();
    clock.increase(80);
    EXPECT_FALSE(timer.isExpired());
    clock.increase(20);
    EXPECT_TRUE(timer.isExpired());
}

TEST_F(TimerTest, UpdateInvokesCallbackOnExpire) {
    int count = 0;
    timer.setDuration(100);
    timer.setCallback([&] { ++count; });
    timer.start();

    clock.increase(50);
    timer.update();
    EXPECT_EQ(count, 0);

    clock.increase(50);
    timer.update();
    EXPECT_EQ(count, 1);
}

TEST_F(TimerTest, UpdateFiresCallbackOnlyOnce) {
    int count = 0;
    timer.setDuration(100);
    timer.setCallback([&] { ++count; });
    timer.start();
    clock.increase(150);

    timer.update();
    timer.update();
    timer.update();
    EXPECT_EQ(count, 1);
}

TEST_F(TimerTest, ResetReArmsCallback) {
    int count = 0;
    timer.setDuration(100);
    timer.setCallback([&] { ++count; });
    timer.start();
    clock.increase(150);
    timer.update();
    EXPECT_EQ(count, 1);

    timer.reset();
    clock.increase(150);
    timer.update();
    EXPECT_EQ(count, 2);
}

TEST_F(TimerTest, UpdateWithoutCallbackIsSafe) {
    timer.setDuration(100);
    timer.start();
    clock.increase(150);
    EXPECT_NO_THROW(timer.update());
}
