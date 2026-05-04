#include "rvc/ObstacleSensorSubject.hpp"

#include <cstdint>
#include <memory>

#include <gtest/gtest.h>

using rvc::ObstacleSensorSubject;

class ObstacleSensorTest : public ::testing::Test {
protected:
    class MockSensor : public rvc::IObstacleSensor {
    public:
        bool initialize() override {
            return true;
        }

        bool isFrontDetected() override {
            return front_;
        }

        bool isLeftDetected() override {
            return left_;
        }

        bool isRightDetected() override {
            return right_;
        }

        void setObstacles(bool f, bool l, bool r) {
            front_ = f;
            left_ = l;
            right_ = r;
        }

    private:
        bool front_{false};
        bool left_{false};
        bool right_{false};
    };

    class MockObserver : public rvc::ISensorObserver {
    public:
        void onObstacleDetected(bool front, bool left, bool right) override {
            lastFront_ = front;
            lastLeft_ = left;
            lastRight_ = right;
            updateCount_++;
        }

        // cppcheck-suppress unusedFunction
        void onDustDetected(bool /*detected*/) override {
        }

        bool lastFront() const {
            return lastFront_;
        }

        bool lastLeft() const {
            return lastLeft_;
        }

        bool lastRight() const {
            return lastRight_;
        }

        int updateCount() const {
            return updateCount_;
        }

    private:
        bool lastFront_{false};
        bool lastLeft_{false};
        bool lastRight_{false};
        int updateCount_{0};
    };

    // Helper to get raw pointer for setting states before moving to subject
    MockSensor* rawFrontSensor;
    MockSensor* rawLeftSensor;
    MockSensor* rawRightSensor;
    std::unique_ptr<MockSensor> frontSensor;
    std::unique_ptr<MockSensor> leftSensor;
    std::unique_ptr<MockSensor> rightSensor;

    // cppcheck-suppress unusedFunction
    void SetUp() override {
        frontSensor = std::make_unique<MockSensor>();
        leftSensor = std::make_unique<MockSensor>();
        rightSensor = std::make_unique<MockSensor>();
        rawFrontSensor = frontSensor.get();
        rawLeftSensor = leftSensor.get();
        rawRightSensor = rightSensor.get();
    }
};

TEST_F(ObstacleSensorTest, ObserverReceivesUpdates) {
    ObstacleSensorSubject subject{std::move(frontSensor), std::move(leftSensor),
                                  std::move(rightSensor)};
    MockObserver observer;
    subject.attach(&observer);

    // Initially no obstacle
    EXPECT_FALSE(observer.lastFront());
    EXPECT_EQ(observer.updateCount(), 0);

    // Notify observer multiple times
    subject.notify();
    EXPECT_EQ(observer.updateCount(), 1);
    EXPECT_FALSE(observer.lastFront());

    // Set obstacle and notify
    rawFrontSensor->setObstacles(true, false, false);
    rawLeftSensor->setObstacles(false, false, false);
    rawRightSensor->setObstacles(false, false, true);
    subject.notify();
    EXPECT_EQ(observer.updateCount(), 2);
    EXPECT_TRUE(observer.lastFront());
    EXPECT_FALSE(observer.lastLeft());
    EXPECT_TRUE(observer.lastRight());
}

TEST_F(ObstacleSensorTest, DetachObserver) {
    ObstacleSensorSubject subject{std::move(frontSensor), std::move(leftSensor),
                                  std::move(rightSensor)};
    MockObserver observer;
    subject.attach(&observer);
    subject.detach(&observer);

    // Simulate obstacle detection
    subject.notify();
    EXPECT_EQ(observer.updateCount(), 0); // Observer should not receive update
}

TEST_F(ObstacleSensorTest, AttachMultipleObservers) {
    ObstacleSensorSubject subject{std::move(frontSensor), std::move(leftSensor),
                                  std::move(rightSensor)};
    MockObserver observer1;

    class MockObserver2 : public rvc::ISensorObserver {
    public:
        void onObstacleDetected(bool /*front*/, bool /*left*/, bool /*right*/) override {
            updateCount_++;
        }

        void onDustDetected(bool /*detected*/) override {
        }

        int updateCount() const {
            return updateCount_;
        }

    private:
        int updateCount_{0};
    };

    MockObserver2 observer2;
    subject.attach(&observer1);
    subject.attach(&observer2);

    // Both observers should receive notification
    subject.notify();
    EXPECT_EQ(observer1.updateCount(), 1);
    EXPECT_EQ(observer2.updateCount(), 1);

    // Detach first observer
    subject.detach(&observer1);
    subject.notify();

    // Only second observer should receive update
    EXPECT_EQ(observer1.updateCount(), 1); // No change
    EXPECT_EQ(observer2.updateCount(), 2); // Updated
}

TEST_F(ObstacleSensorTest, AttachIgnoresNullptrAndDuplicates) {
    ObstacleSensorSubject subject{std::move(frontSensor), std::move(leftSensor),
                                  std::move(rightSensor)};
    MockObserver observer;

    subject.attach(nullptr);
    subject.attach(&observer);
    subject.attach(&observer);

    subject.notify();

    EXPECT_EQ(observer.updateCount(), 1);
}

TEST_F(ObstacleSensorTest, DetachIgnoresNullptr) {
    ObstacleSensorSubject subject{std::move(frontSensor), std::move(leftSensor),
                                  std::move(rightSensor)};
    MockObserver observer;

    subject.attach(&observer);
    subject.detach(nullptr);
    subject.notify();

    EXPECT_EQ(observer.updateCount(), 1);
}

TEST_F(ObstacleSensorTest, PollCanBeCalledSafely) {
    ObstacleSensorSubject subject{std::move(frontSensor), std::move(leftSensor),
                                  std::move(rightSensor)};
    MockObserver observer;
    subject.attach(&observer);

    subject.poll();

    // Timer is not expired by default, so poll should not notify yet.
    EXPECT_EQ(observer.updateCount(), 0);
}
