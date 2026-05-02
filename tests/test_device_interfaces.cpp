#include "rvc/i_cleaner.hpp"
#include "rvc/i_dust_sensor.hpp"
#include "rvc/i_motor.hpp"
#include "rvc/i_obstacle_sensor.hpp"
#include "rvc/types.hpp"

#include <gtest/gtest.h>

namespace {
class TestMotor : public rvc::IMotor {
public:
    bool initialize() override {
        isInitialized = true;
        return true;
    }

    void move(rvc::Direction d) override {
        lastDirection = d;
    }

    bool isInitialized{false};
    rvc::Direction lastDirection{rvc::Direction::STOP};
};

class TestCleaner : public rvc::ICleaner {
public:
    bool initialize() override {
        isInitialized = true;
        return true;
    }

    void setPower(rvc::PowerLevel level) override {
        lastLevel = level;
    }

    bool isInitialized{false};
    rvc::PowerLevel lastLevel{rvc::PowerLevel::OFF};
};

class TestObstacleSensor : public rvc::IObstacleSensor {
public:
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

    bool front{false};
    bool left{false};
    bool right{false};
};

class TestDustSensor : public rvc::IDustSensor {
public:
    bool initialize() override {
        return true;
    }

    bool isDustDetected() override {
        return dust;
    }

    bool dust{false};
};
} // namespace

TEST(DeviceInterfacesTest, MotorAcceptsDirection) {
    TestMotor m;
    EXPECT_TRUE(m.initialize());
    EXPECT_TRUE(m.isInitialized);
    m.move(rvc::Direction::FORWARD);
    EXPECT_EQ(m.lastDirection, rvc::Direction::FORWARD);
    m.move(rvc::Direction::BACKWARD);
    EXPECT_EQ(m.lastDirection, rvc::Direction::BACKWARD);
}

TEST(DeviceInterfacesTest, MotorDefaultIsStop) {
    TestMotor m;
    EXPECT_EQ(m.lastDirection, rvc::Direction::STOP);
    EXPECT_FALSE(m.isInitialized);
}

TEST(DeviceInterfacesTest, MotorCoversAllDirections) {
    TestMotor m;
    for (auto d : {rvc::Direction::FORWARD, rvc::Direction::BACKWARD, rvc::Direction::LEFT,
                   rvc::Direction::RIGHT, rvc::Direction::STOP}) {
        m.move(d);
        EXPECT_EQ(m.lastDirection, d);
    }
}

TEST(DeviceInterfacesTest, MotorOverwritesLastDirection) {
    TestMotor m;
    m.move(rvc::Direction::LEFT);
    m.move(rvc::Direction::RIGHT);
    EXPECT_EQ(m.lastDirection, rvc::Direction::RIGHT);
}

TEST(DeviceInterfacesTest, MotorThroughInterfacePointer) {
    TestMotor m;
    rvc::IMotor* iface = &m;
    EXPECT_TRUE(iface->initialize());
    iface->move(rvc::Direction::FORWARD);
    EXPECT_EQ(m.lastDirection, rvc::Direction::FORWARD);
}

TEST(DeviceInterfacesTest, CleanerInitializeSetsFlag) {
    TestCleaner c;
    EXPECT_FALSE(c.isInitialized);
    EXPECT_TRUE(c.initialize());
    EXPECT_TRUE(c.isInitialized);
}

TEST(DeviceInterfacesTest, CleanerDefaultPowerOff) {
    TestCleaner c;
    EXPECT_EQ(c.lastLevel, rvc::PowerLevel::OFF);
}

TEST(DeviceInterfacesTest, CleanerCoversAllPowerLevels) {
    TestCleaner c;
    for (auto level : {rvc::PowerLevel::OFF, rvc::PowerLevel::NORMAL, rvc::PowerLevel::POWER_UP}) {
        c.setPower(level);
        EXPECT_EQ(c.lastLevel, level);
    }
}

TEST(DeviceInterfacesTest, CleanerThroughInterfacePointer) {
    TestCleaner c;
    rvc::ICleaner* iface = &c;
    iface->setPower(rvc::PowerLevel::POWER_UP);
    EXPECT_EQ(c.lastLevel, rvc::PowerLevel::POWER_UP);
}

TEST(DeviceInterfacesTest, ObstacleSensorDefaultsClear) {
    TestObstacleSensor s;
    EXPECT_TRUE(s.initialize());
    EXPECT_FALSE(s.isFrontDetected());
    EXPECT_FALSE(s.isLeftDetected());
    EXPECT_FALSE(s.isRightDetected());
}

TEST(DeviceInterfacesTest, ObstacleSensorIndependentSides) {
    TestObstacleSensor s;
    s.front = true;
    EXPECT_TRUE(s.isFrontDetected());
    EXPECT_FALSE(s.isLeftDetected());
    EXPECT_FALSE(s.isRightDetected());

    s.front = false;
    s.left = true;
    EXPECT_FALSE(s.isFrontDetected());
    EXPECT_TRUE(s.isLeftDetected());
    EXPECT_FALSE(s.isRightDetected());

    s.left = false;
    s.right = true;
    EXPECT_FALSE(s.isFrontDetected());
    EXPECT_FALSE(s.isLeftDetected());
    EXPECT_TRUE(s.isRightDetected());
}

TEST(DeviceInterfacesTest, ObstacleSensorAllDetected) {
    TestObstacleSensor s;
    s.front = s.left = s.right = true;
    EXPECT_TRUE(s.isFrontDetected());
    EXPECT_TRUE(s.isLeftDetected());
    EXPECT_TRUE(s.isRightDetected());
}

TEST(DeviceInterfacesTest, ObstacleSensorThroughInterfacePointer) {
    TestObstacleSensor s;
    rvc::IObstacleSensor* iface = &s;
    s.left = true;
    EXPECT_TRUE(iface->isLeftDetected());
    EXPECT_FALSE(iface->isFrontDetected());
}

TEST(DeviceInterfacesTest, DustSensorInitialize) {
    TestDustSensor d;
    EXPECT_TRUE(d.initialize());
}

TEST(DeviceInterfacesTest, DustSensorReportsState) {
    TestDustSensor d;
    EXPECT_FALSE(d.isDustDetected());
    d.dust = true;
    EXPECT_TRUE(d.isDustDetected());
    d.dust = false;
    EXPECT_FALSE(d.isDustDetected());
}

TEST(DeviceInterfacesTest, DustSensorThroughInterfacePointer) {
    TestDustSensor d;
    rvc::IDustSensor* iface = &d;
    d.dust = true;
    EXPECT_TRUE(iface->isDustDetected());
}

TEST(DeviceInterfacesTest, CleanerAcceptsPowerLevel) {
    TestCleaner c;
    EXPECT_TRUE(c.initialize());
    c.setPower(rvc::PowerLevel::NORMAL);
    EXPECT_EQ(c.lastLevel, rvc::PowerLevel::NORMAL);
    c.setPower(rvc::PowerLevel::POWER_UP);
    EXPECT_EQ(c.lastLevel, rvc::PowerLevel::POWER_UP);
}

TEST(DeviceInterfacesTest, ObstacleSensorReportsAxesIndependently) {
    TestObstacleSensor s;
    EXPECT_FALSE(s.isFrontDetected());
    EXPECT_FALSE(s.isLeftDetected());
    EXPECT_FALSE(s.isRightDetected());

    s.front = true;
    s.right = true;
    EXPECT_TRUE(s.isFrontDetected());
    EXPECT_FALSE(s.isLeftDetected());
    EXPECT_TRUE(s.isRightDetected());
}

TEST(DeviceInterfacesTest, DustSensorReports) {
    TestDustSensor s;
    EXPECT_FALSE(s.isDustDetected());
    s.dust = true;
    EXPECT_TRUE(s.isDustDetected());
}

TEST(DeviceInterfacesTest, InterfacesArePolymorphic) {
    TestMotor m;
    rvc::IMotor* motor = &m;
    motor->move(rvc::Direction::LEFT);
    EXPECT_EQ(m.lastDirection, rvc::Direction::LEFT);

    TestCleaner c;
    rvc::ICleaner* cleaner = &c;
    cleaner->setPower(rvc::PowerLevel::OFF);
    EXPECT_EQ(c.lastLevel, rvc::PowerLevel::OFF);
}
