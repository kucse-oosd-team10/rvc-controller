#include "rvc/i_obstacle_sensor.hpp"
#include "rvc/i_sensor_observer.hpp"
#include "rvc/obstacle_sensor_subject.hpp"

#include <gtest/gtest.h>

namespace {

class MockObstacleSensor : public rvc::IObstacleSensor {
public:
    bool initialize() override {
        return true;
    }

    bool isFrontDetected() override {
        front_reads++;
        return front;
    }

    bool isLeftDetected() override {
        left_reads++;
        return left;
    }

    bool isRightDetected() override {
        right_reads++;
        return right;
    }

    bool front{false};
    bool left{false};
    bool right{false};

    int front_reads{0};
    int left_reads{0};
    int right_reads{0};
};

class MockSensorObserver : public rvc::ISensorObserver {
public:
    void onObstacleDetected(bool f, bool l, bool r) override {
        call_count++;
        last_front = f;
        last_left = l;
        last_right = r;
    }

    void onDustDetected(bool) override {
    }

    int call_count{0};
    bool last_front{false};
    bool last_left{false};
    bool last_right{false};
};

} // namespace

class ObstacleSensorSubjectTest : public ::testing::Test {
protected:
    MockObstacleSensor sensor;
    rvc::ObstacleSensorSubject subject{sensor};
};

TEST_F(ObstacleSensorSubjectTest, InitialStateNoNotifyOnFirstPollIfAllFalse) {
    MockSensorObserver obs;
    subject.attach(&obs);

    subject.poll();

    EXPECT_EQ(obs.call_count, 0);
}

TEST_F(ObstacleSensorSubjectTest, PollNotifiesWhenFrontChanges) {
    MockSensorObserver obs;
    subject.attach(&obs);
    sensor.front = true;

    subject.poll();

    EXPECT_EQ(obs.call_count, 1);
    EXPECT_TRUE(obs.last_front);
    EXPECT_FALSE(obs.last_left);
    EXPECT_FALSE(obs.last_right);
}

TEST_F(ObstacleSensorSubjectTest, PollNotifiesWhenLeftChanges) {
    MockSensorObserver obs;
    subject.attach(&obs);
    sensor.left = true;

    subject.poll();

    EXPECT_EQ(obs.call_count, 1);
    EXPECT_FALSE(obs.last_front);
    EXPECT_TRUE(obs.last_left);
    EXPECT_FALSE(obs.last_right);
}

TEST_F(ObstacleSensorSubjectTest, PollNotifiesWhenRightChanges) {
    MockSensorObserver obs;
    subject.attach(&obs);
    sensor.right = true;

    subject.poll();

    EXPECT_EQ(obs.call_count, 1);
    EXPECT_FALSE(obs.last_front);
    EXPECT_FALSE(obs.last_left);
    EXPECT_TRUE(obs.last_right);
}

TEST_F(ObstacleSensorSubjectTest, PollDoesNotNotifyIfStateUnchanged) {
    MockSensorObserver obs;
    subject.attach(&obs);

    subject.poll();
    subject.poll();
    subject.poll();

    EXPECT_EQ(obs.call_count, 0);
}

TEST_F(ObstacleSensorSubjectTest, PollNotifiesOnceForMultiAxisChange) {
    MockSensorObserver obs;
    subject.attach(&obs);
    sensor.front = true;
    sensor.left = true;
    sensor.right = true;

    subject.poll();

    EXPECT_EQ(obs.call_count, 1);
    EXPECT_TRUE(obs.last_front);
    EXPECT_TRUE(obs.last_left);
    EXPECT_TRUE(obs.last_right);
}

TEST_F(ObstacleSensorSubjectTest, PollNotifiesWhenObstacleClears) {
    MockSensorObserver obs;
    subject.attach(&obs);

    sensor.front = true;
    subject.poll();
    EXPECT_EQ(obs.call_count, 1);

    sensor.front = false;
    subject.poll();

    EXPECT_EQ(obs.call_count, 2);
    EXPECT_FALSE(obs.last_front);
}

TEST_F(ObstacleSensorSubjectTest, OnInterruptNotifiesUnconditionallyEvenAllFalse) {
    MockSensorObserver obs;
    subject.attach(&obs);

    subject.onInterrupt();

    EXPECT_EQ(obs.call_count, 1);
    EXPECT_FALSE(obs.last_front);
    EXPECT_FALSE(obs.last_left);
    EXPECT_FALSE(obs.last_right);
}

TEST_F(ObstacleSensorSubjectTest, OnInterruptReadsLatestSensorValues) {
    MockSensorObserver obs;
    subject.attach(&obs);
    sensor.front = true;
    sensor.left = false;
    sensor.right = false;

    subject.onInterrupt();

    EXPECT_EQ(obs.call_count, 1);
    EXPECT_TRUE(obs.last_front);
    EXPECT_FALSE(obs.last_left);
    EXPECT_FALSE(obs.last_right);
}

TEST_F(ObstacleSensorSubjectTest, OnInterruptUpdatesCacheSoNextPollDoesNotRefire) {
    MockSensorObserver obs;
    subject.attach(&obs);
    sensor.front = true;
    sensor.left = true;
    sensor.right = true;

    subject.onInterrupt();
    EXPECT_EQ(obs.call_count, 1);

    // Cache must now reflect (true, true, true). Poll with same values must not notify.
    subject.poll();
    EXPECT_EQ(obs.call_count, 1);
}

TEST_F(ObstacleSensorSubjectTest, MultipleObserversNotified) {
    MockSensorObserver obs1;
    MockSensorObserver obs2;
    subject.attach(&obs1);
    subject.attach(&obs2);
    sensor.front = true;

    subject.poll();

    EXPECT_EQ(obs1.call_count, 1);
    EXPECT_TRUE(obs1.last_front);
    EXPECT_EQ(obs2.call_count, 1);
    EXPECT_TRUE(obs2.last_front);
}

TEST_F(ObstacleSensorSubjectTest, DetachedObserverNotNotified) {
    MockSensorObserver obs;
    subject.attach(&obs);
    subject.detach(&obs);
    sensor.front = true;

    subject.poll();

    EXPECT_EQ(obs.call_count, 0);
}

TEST_F(ObstacleSensorSubjectTest, DuplicateAttachHasNoEffect) {
    MockSensorObserver obs;
    subject.attach(&obs);
    subject.attach(&obs);
    sensor.front = true;

    subject.poll();

    EXPECT_EQ(obs.call_count, 1);
}

TEST_F(ObstacleSensorSubjectTest, AttachNullptrIsNoop) {
    subject.attach(nullptr);
    sensor.front = true;

    subject.poll();
    // No crash, nothing to assert beyond reaching here.
    SUCCEED();
}

TEST_F(ObstacleSensorSubjectTest, NotifyMethodEmitsCachedState) {
    MockSensorObserver obs;
    subject.attach(&obs);
    sensor.front = true;
    sensor.left = true;
    subject.poll();
    EXPECT_EQ(obs.call_count, 1);

    // Explicit notify() re-emits with the same cached state.
    subject.notify();

    EXPECT_EQ(obs.call_count, 2);
    EXPECT_TRUE(obs.last_front);
    EXPECT_TRUE(obs.last_left);
    EXPECT_FALSE(obs.last_right);
}
