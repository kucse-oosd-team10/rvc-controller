#include "rvc/i_avoid_strategy.hpp"
#include "rvc/i_rvc_state.hpp"
#include "rvc/i_sensor_observer.hpp"
#include "rvc/types.hpp"

#include <vector>

#include <gtest/gtest.h>

namespace {

class TestObserver : public rvc::ISensorObserver {
public:
    void onObstacleDetected(bool front, bool left, bool right) override {
        ++obstacleCount;
        lastFront = front;
        lastLeft = left;
        lastRight = right;
    }

    void onDustDetected(bool detected) override {
        ++dustCount;
        lastDust = detected;
    }

    int obstacleCount{0};
    int dustCount{0};
    bool lastFront{false};
    bool lastLeft{false};
    bool lastRight{false};
    bool lastDust{false};
};

class TestSubject : public rvc::ISensorSubject {
public:
    void attach(rvc::ISensorObserver* obs) override {
        observers_.push_back(obs);
    }

    void detach(rvc::ISensorObserver* obs) override {
        std::erase(observers_, obs);
    }

    void notify() override {
        for (auto* o : observers_) {
            o->onObstacleDetected(true, false, true);
        }
    }

private:
    std::vector<rvc::ISensorObserver*> observers_;
};

class TestState : public rvc::IRVCState {
public:
    void onEnter(rvc::RVCController& /*ctx*/) override {
        ++enterCount;
    }

    void onExit(rvc::RVCController& /*ctx*/) override {
        ++exitCount;
    }

    void handleObstacle(rvc::RVCController& /*ctx*/, bool /*front*/, bool /*left*/,
                        bool /*right*/) override {
        ++obstacleCount;
    }

    void handleDust(rvc::RVCController& /*ctx*/, bool /*detected*/) override {
        ++dustCount;
    }

    void handlePowerOff(rvc::RVCController& /*ctx*/) override {
        ++powerOffCount;
    }

    int enterCount{0};
    int exitCount{0};
    int obstacleCount{0};
    int dustCount{0};
    int powerOffCount{0};
};

class LeftPriorityStrategy : public rvc::IAvoidStrategy {
public:
    rvc::Direction decideDirection(bool front, bool left, bool right) override {
        if (front && left && right) {
            return rvc::Direction::BACKWARD;
        }
        if (front && left) {
            return rvc::Direction::RIGHT;
        }
        if (front) {
            return rvc::Direction::LEFT;
        }
        return rvc::Direction::FORWARD;
    }

    bool needsReverse(bool front, bool left, bool right) override {
        return front && left && right;
    }
};

} // namespace

TEST(SensorSubjectTest, AttachThenNotifyDeliversToObserver) {
    TestSubject subject;
    TestObserver obs;
    subject.attach(&obs);
    subject.notify();
    EXPECT_EQ(obs.obstacleCount, 1);
    EXPECT_TRUE(obs.lastFront);
    EXPECT_FALSE(obs.lastLeft);
    EXPECT_TRUE(obs.lastRight);
}

TEST(SensorSubjectTest, MultipleObserversAllReceive) {
    TestSubject subject;
    TestObserver a;
    TestObserver b;
    subject.attach(&a);
    subject.attach(&b);
    subject.notify();
    EXPECT_EQ(a.obstacleCount, 1);
    EXPECT_EQ(b.obstacleCount, 1);
}

TEST(SensorSubjectTest, DetachStopsDelivery) {
    TestSubject subject;
    TestObserver obs;
    subject.attach(&obs);
    subject.detach(&obs);
    subject.notify();
    EXPECT_EQ(obs.obstacleCount, 0);
}

TEST(IRVCStateTest, FakeStateImplementsAllSlots) {
    TestState state;
    EXPECT_EQ(state.enterCount, 0);
    EXPECT_EQ(state.exitCount, 0);
    EXPECT_EQ(state.obstacleCount, 0);
    EXPECT_EQ(state.dustCount, 0);
    EXPECT_EQ(state.powerOffCount, 0);
}

TEST(IAvoidStrategyTest, LeftPriorityDecisions) {
    LeftPriorityStrategy s;
    EXPECT_EQ(s.decideDirection(false, false, false), rvc::Direction::FORWARD);
    EXPECT_EQ(s.decideDirection(true, false, false), rvc::Direction::LEFT);
    EXPECT_EQ(s.decideDirection(true, true, false), rvc::Direction::RIGHT);
    EXPECT_EQ(s.decideDirection(true, true, true), rvc::Direction::BACKWARD);
}

TEST(IAvoidStrategyTest, NeedsReverseOnlyWhenAllBlocked) {
    LeftPriorityStrategy s;
    EXPECT_FALSE(s.needsReverse(true, true, false));
    EXPECT_FALSE(s.needsReverse(true, false, true));
    EXPECT_TRUE(s.needsReverse(true, true, true));
}

TEST(IAvoidStrategyTest, PolymorphicDispatch) {
    LeftPriorityStrategy s;
    rvc::IAvoidStrategy* base = &s;
    EXPECT_EQ(base->decideDirection(true, false, false), rvc::Direction::LEFT);
}
