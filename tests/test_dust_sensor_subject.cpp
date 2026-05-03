#include "rvc/dust_sensor_subject.hpp"
#include "rvc/i_dust_sensor.hpp"
#include "rvc/i_sensor_observer.hpp"

#include <gtest/gtest.h>

namespace {

class MockDustSensor : public rvc::IDustSensor {
public:
    bool initialize() override {
        return true;
    }

    bool isDustDetected() override {
        return dust_detected;
    }

    bool dust_detected{false};
};

class MockSensorObserver : public rvc::ISensorObserver {
public:
    void onObstacleDetected(bool, bool, bool) override {
    }

    void onDustDetected(bool detected) override {
        call_count++;
        last_detected = detected;
    }

    int call_count{0};
    bool last_detected{false};
};

} // namespace

class DustSensorSubjectTest : public ::testing::Test {
protected:
    MockDustSensor sensor;
    rvc::DustSensorSubject subject{&sensor};
    MockSensorObserver observer1;
    MockSensorObserver observer2;
};

TEST_F(DustSensorSubjectTest, InitialStateIsNoDust) {
    EXPECT_FALSE(subject.isDustDetected());
}

TEST_F(DustSensorSubjectTest, PollNotifiesWhenDustDetected) {
    subject.attach(&observer1);
    sensor.dust_detected = true;

    subject.poll();

    EXPECT_TRUE(subject.isDustDetected());
    EXPECT_EQ(observer1.call_count, 1);
    EXPECT_TRUE(observer1.last_detected);
}

TEST_F(DustSensorSubjectTest, PollDoesNotNotifyIfStateUnchanged) {
    subject.attach(&observer1);
    sensor.dust_detected = false;

    subject.poll();

    EXPECT_FALSE(subject.isDustDetected());
    EXPECT_EQ(observer1.call_count, 0);
}

TEST_F(DustSensorSubjectTest, PollNotifiesWhenDustCleared) {
    subject.attach(&observer1);

    // First detect dust
    sensor.dust_detected = true;
    subject.poll();
    EXPECT_EQ(observer1.call_count, 1);

    // Then clear dust
    sensor.dust_detected = false;
    subject.poll();

    EXPECT_FALSE(subject.isDustDetected());
    EXPECT_EQ(observer1.call_count, 2);
    EXPECT_FALSE(observer1.last_detected);
}

TEST_F(DustSensorSubjectTest, MultipleObserversNotified) {
    subject.attach(&observer1);
    subject.attach(&observer2);
    sensor.dust_detected = true;

    subject.poll();

    EXPECT_EQ(observer1.call_count, 1);
    EXPECT_TRUE(observer1.last_detected);
    EXPECT_EQ(observer2.call_count, 1);
    EXPECT_TRUE(observer2.last_detected);
}

TEST_F(DustSensorSubjectTest, DetachedObserverNotNotified) {
    subject.attach(&observer1);
    subject.detach(&observer1);
    sensor.dust_detected = true;

    subject.poll();

    EXPECT_EQ(observer1.call_count, 0);
}

TEST_F(DustSensorSubjectTest, DuplicateAttachHasNoEffect) {
    subject.attach(&observer1);
    subject.attach(&observer1);
    sensor.dust_detected = true;

    subject.poll();

    EXPECT_EQ(observer1.call_count, 1);
}

TEST_F(DustSensorSubjectTest, NotifyMethodNotifiesObserversExplicitly) {
    subject.attach(&observer1);
    // Note: notify() uses the current dustDetected_ state, not sensor state.
    // dustDetected_ is updated by poll().

    sensor.dust_detected = true;
    subject.poll(); // notifies once
    EXPECT_EQ(observer1.call_count, 1);

    subject.notify(); // notifies again with same state
    EXPECT_EQ(observer1.call_count, 2);
    EXPECT_TRUE(observer1.last_detected);
}

TEST_F(DustSensorSubjectTest, PollWithNullSensorDoesNothing) {
    rvc::DustSensorSubject nullSubject{nullptr};
    nullSubject.attach(&observer1);

    // This should not crash and should not notify
    nullSubject.poll();

    EXPECT_EQ(observer1.call_count, 0);
    EXPECT_FALSE(nullSubject.isDustDetected());
}
