#include "rvc/i_avoid_strategy.hpp"
#include "rvc/i_rvc_state.hpp"
#include "rvc/i_sensor_observer.hpp"
#include "rvc/types.hpp"

#include <memory>
#include <vector>

#include <gtest/gtest.h>

namespace rvc {
class RVCController {};
} // namespace rvc

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

    void notifyDust(bool detected) {
        for (auto* o : observers_) {
            o->onDustDetected(detected);
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

// attach 후 notify 시 observer에 장애물 이벤트가 전달되는지 확인
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

// 여러 observer 등록 시 모두 알림을 수신하는지 확인
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

// detach 후 notify 시 해당 observer에 이벤트가 전달되지 않는지 확인
TEST(SensorSubjectTest, DetachStopsDelivery) {
    TestSubject subject;
    TestObserver obs;
    subject.attach(&obs);
    subject.detach(&obs);
    subject.notify();
    EXPECT_EQ(obs.obstacleCount, 0);
}

// notifyDust 호출 시 observer의 onDustDetected가 올바르게 전달되는지 확인
TEST(SensorSubjectTest, NotifyDustDeliversDustEvent) {
    TestSubject subject;
    TestObserver obs;
    subject.attach(&obs);
    subject.notifyDust(true);
    EXPECT_EQ(obs.dustCount, 1);
    EXPECT_TRUE(obs.lastDust);
}

// dust 미감지 상태도 observer에 정확히 전달되는지 확인
TEST(SensorSubjectTest, NotifyDustFalseIsDelivered) {
    TestSubject subject;
    TestObserver obs;
    subject.attach(&obs);
    subject.notifyDust(false);
    EXPECT_EQ(obs.dustCount, 1);
    EXPECT_FALSE(obs.lastDust);
}

// observer가 없을 때 notify 호출이 안전하게 no-op으로 처리되는지 확인
TEST(SensorSubjectTest, NotifyWithNoObserversIsNoOp) {
    TestSubject subject;
    subject.notify(); // must not crash
}

// 등록되지 않은 observer를 detach해도 기존 observer에 영향 없는지 확인
TEST(SensorSubjectTest, DetachUnregisteredObserverIsNoOp) {
    TestSubject subject;
    TestObserver a;
    TestObserver b;
    subject.attach(&a);
    subject.detach(&b); // b is not attached
    subject.notify();
    EXPECT_EQ(a.obstacleCount, 1);
}

// attach/detach 반복 후 최종 등록된 observer만 알림을 수신하는지 확인
TEST(SensorSubjectTest, ReattachAfterDetachReceivesNotification) {
    TestSubject subject;
    TestObserver obs;
    subject.attach(&obs);
    subject.detach(&obs);
    subject.attach(&obs);
    subject.notify();
    EXPECT_EQ(obs.obstacleCount, 1);
}

// ISensorObserver 가상 소멸자가 파생 클래스에서 안전하게 호출되는지 확인
TEST(SensorSubjectTest, ObserverVirtualDestructorViaUniquePtr) {
    auto obs = std::make_unique<TestObserver>();
    std::unique_ptr<rvc::ISensorObserver> base = std::move(obs);
    // 소멸 시 UB 없이 정상 종료되어야 함
}

// ISensorSubject 가상 소멸자가 파생 클래스에서 안전하게 호출되는지 확인
TEST(SensorSubjectTest, SubjectVirtualDestructorViaUniquePtr) {
    auto subject = std::make_unique<TestSubject>();
    std::unique_ptr<rvc::ISensorSubject> base = std::move(subject);
}

// TestState 초기 카운터가 모두 0인지 확인
TEST(IRVCStateTest, FakeStateImplementsAllSlots) {
    TestState state;
    EXPECT_EQ(state.enterCount, 0);
    EXPECT_EQ(state.exitCount, 0);
    EXPECT_EQ(state.obstacleCount, 0);
    EXPECT_EQ(state.dustCount, 0);
    EXPECT_EQ(state.powerOffCount, 0);
}

// onEnter 호출 시 enterCount가 증가하는지 확인
TEST(IRVCStateTest, OnEnterIncrementsCounter) {
    TestState state;
    rvc::RVCController ctx;
    state.onEnter(ctx);
    EXPECT_EQ(state.enterCount, 1);
}

// onExit 호출 시 exitCount가 증가하는지 확인
TEST(IRVCStateTest, OnExitIncrementsCounter) {
    TestState state;
    rvc::RVCController ctx;
    state.onExit(ctx);
    EXPECT_EQ(state.exitCount, 1);
}

// handleObstacle 호출 시 obstacleCount가 증가하는지 확인
TEST(IRVCStateTest, HandleObstacleIncrementsCounter) {
    TestState state;
    rvc::RVCController ctx;
    state.handleObstacle(ctx, true, false, false);
    EXPECT_EQ(state.obstacleCount, 1);
}

// handleDust 호출 시 dustCount가 증가하는지 확인
TEST(IRVCStateTest, HandleDustIncrementsCounter) {
    TestState state;
    rvc::RVCController ctx;
    state.handleDust(ctx, true);
    EXPECT_EQ(state.dustCount, 1);
}

// handlePowerOff 호출 시 powerOffCount가 증가하는지 확인
TEST(IRVCStateTest, HandlePowerOffIncrementsCounter) {
    TestState state;
    rvc::RVCController ctx;
    state.handlePowerOff(ctx);
    EXPECT_EQ(state.powerOffCount, 1);
}

// 각 핸들러를 여러 번 호출 시 카운터가 누적되는지 확인
TEST(IRVCStateTest, MultipleCallsAccumulateCounts) {
    TestState state;
    rvc::RVCController ctx;
    state.onEnter(ctx);
    state.onEnter(ctx);
    state.handleDust(ctx, false);
    EXPECT_EQ(state.enterCount, 2);
    EXPECT_EQ(state.dustCount, 1);
}

// IRVCState 가상 소멸자가 파생 클래스에서 안전하게 호출되는지 확인
TEST(IRVCStateTest, VirtualDestructorViaUniquePtr) {
    auto state = std::make_unique<TestState>();
    std::unique_ptr<rvc::IRVCState> base = std::move(state);
}

// 장애물 조합별 회피 방향이 우선순위 전략에 맞게 결정되는지 확인
TEST(IAvoidStrategyTest, LeftPriorityDecisions) {
    LeftPriorityStrategy s;
    EXPECT_EQ(s.decideDirection(false, false, false), rvc::Direction::FORWARD);
    EXPECT_EQ(s.decideDirection(true, false, false), rvc::Direction::LEFT);
    EXPECT_EQ(s.decideDirection(true, true, false), rvc::Direction::RIGHT);
    EXPECT_EQ(s.decideDirection(true, true, true), rvc::Direction::BACKWARD);
}

// 전방+우측 장애물 시 좌회전(LEFT)이 선택되는지 확인
TEST(IAvoidStrategyTest, FrontAndRightBlockedChoosesLeft) {
    LeftPriorityStrategy s;
    EXPECT_EQ(s.decideDirection(true, false, true), rvc::Direction::LEFT);
}

// 장애물 없을 때 needsReverse가 false인지 확인
TEST(IAvoidStrategyTest, NeedsReverseReturnsFalseWhenClear) {
    LeftPriorityStrategy s;
    EXPECT_FALSE(s.needsReverse(false, false, false));
}

// 전방만 막혔을 때 needsReverse가 false인지 확인
TEST(IAvoidStrategyTest, NeedsReverseReturnsFalseWhenOnlyFront) {
    LeftPriorityStrategy s;
    EXPECT_FALSE(s.needsReverse(true, false, false));
}

// 세 방향 모두 막혔을 때만 needsReverse가 true인지 확인
TEST(IAvoidStrategyTest, NeedsReverseOnlyWhenAllBlocked) {
    LeftPriorityStrategy s;
    EXPECT_FALSE(s.needsReverse(true, true, false));
    EXPECT_FALSE(s.needsReverse(true, false, true));
    EXPECT_TRUE(s.needsReverse(true, true, true));
}

// 인터페이스 포인터를 통해 다형성 디스패치가 올바르게 동작하는지 확인
TEST(IAvoidStrategyTest, PolymorphicDispatch) {
    LeftPriorityStrategy s;
    rvc::IAvoidStrategy* base = &s;
    EXPECT_EQ(base->decideDirection(true, false, false), rvc::Direction::LEFT);
}

// IAvoidStrategy 가상 소멸자가 파생 클래스에서 안전하게 호출되는지 확인
TEST(IAvoidStrategyTest, VirtualDestructorViaUniquePtr) {
    auto strategy = std::make_unique<LeftPriorityStrategy>();
    std::unique_ptr<rvc::IAvoidStrategy> base = std::move(strategy);
}
