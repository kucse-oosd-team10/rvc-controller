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

// 갓 생성된 타이머가 동작 중도 만료 상태도 아님을 확인한다.
TEST_F(TimerTest, NoRunInit) {
    EXPECT_FALSE(timer.isRunning());
    EXPECT_FALSE(timer.isExpired());
}

// start() 호출 시 타이머가 동작 상태로 전환되며 즉시 만료되지 않음을 확인한다.
TEST_F(TimerTest, StartMakeRun) {
    timer.setDuration(100);
    timer.start();
    EXPECT_TRUE(timer.isRunning());
    EXPECT_FALSE(timer.isExpired());
}

// stop() 호출 시 동작 중인 타이머가 정지되어 isRunning이 false가 됨을 확인한다.
TEST_F(TimerTest, StopIsStop) {
    timer.start();
    EXPECT_TRUE(timer.isRunning());

    timer.stop();
    EXPECT_FALSE(timer.isRunning());
}

// 경과 시간이 duration에 도달하는 정확한 경계에서 isExpired가 true로 바뀌는지 확인한다.
TEST_F(TimerTest, CheckExpire) {
    timer.setDuration(100);
    timer.start();
    clock.increase(99);
    EXPECT_FALSE(timer.isExpired());
    clock.increase(1);
    EXPECT_TRUE(timer.isExpired());
}

// 정지된 타이머는 duration을 초과해도 만료로 보고하지 않음을 확인한다.
TEST_F(TimerTest, StoppedTimerNotExpired) {
    timer.setDuration(100);
    timer.start();
    clock.increase(200);
    timer.stop();
    EXPECT_FALSE(timer.isExpired());
}

// reset() 호출 시 현재 시각을 새 기준점으로 카운트다운이 다시 시작되는지 확인한다.
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

// update()가 만료 시점에 등록된 콜백을 호출하는지 확인한다.
TEST_F(TimerTest, UpdateInvokesCallbackOnExpire) {
    int count = 0;
    timer.setDuration(100);
    timer.setCallback([&] {
        ++count;
    });
    timer.start();

    clock.increase(50);
    timer.update();
    EXPECT_EQ(count, 0);

    clock.increase(50);
    timer.update();
    EXPECT_EQ(count, 1);
}

// 만료 후 update()를 여러 번 호출해도 콜백은 단 한 번만 발생함을 확인한다.
TEST_F(TimerTest, UpdateFiresCallbackOnlyOnce) {
    int count = 0;
    timer.setDuration(100);
    timer.setCallback([&] {
        ++count;
    });
    timer.start();
    clock.increase(150);

    timer.update();
    timer.update();
    timer.update();
    EXPECT_EQ(count, 1);
}

// reset() 호출 시 일회성 콜백이 재무장되어 다음 만료에서 다시 발화하는지 확인한다.
TEST_F(TimerTest, ResetReArmsCallback) {
    int count = 0;
    timer.setDuration(100);
    timer.setCallback([&] {
        ++count;
    });
    timer.start();
    clock.increase(150);
    timer.update();
    EXPECT_EQ(count, 1);

    timer.reset();
    clock.increase(150);
    timer.update();
    EXPECT_EQ(count, 2);
}

// 콜백이 등록되지 않은 상태에서 update()를 호출해도 안전한지 확인한다.
TEST_F(TimerTest, UpdateWithoutCallbackIsSafe) {
    timer.setDuration(100);
    timer.start();
    clock.increase(150);
    EXPECT_NO_THROW(timer.update());
}

// 기본 생성자로 만든 Timer가 실제 시스템 클록(defaultClock) 경로를 사용해 정상 동작하는지 확인한다.
TEST(TimerDefaultClockTest, UsesDefaultSystemClock) {
    Timer t;
    t.setDuration(0);
    EXPECT_FALSE(t.isRunning());
    t.start();
    EXPECT_TRUE(t.isRunning());
    EXPECT_TRUE(t.isExpired());
    t.update();
    t.stop();
    EXPECT_FALSE(t.isRunning());
}

// 시작하지 않은 타이머에 update()를 호출해도 콜백이 발화하지 않음을 확인한다.
TEST_F(TimerTest, UpdateOnIdleTimerDoesNothing) {
    int count = 0;
    timer.setDuration(100);
    timer.setCallback([&] {
        ++count;
    });
    clock.increase(500);
    timer.update();
    EXPECT_EQ(count, 0);
    EXPECT_FALSE(timer.isRunning());
}

// 동작 중에 setDuration()으로 duration을 변경하면 다음 만료 판단부터 새 값이 적용되는지 확인한다.
TEST_F(TimerTest, DurationCanBeReassigned) {
    timer.setDuration(50);
    timer.start();
    clock.increase(30);
    timer.setDuration(200);
    clock.increase(30);
    EXPECT_FALSE(timer.isExpired());
    clock.increase(140);
    EXPECT_TRUE(timer.isExpired());
}
