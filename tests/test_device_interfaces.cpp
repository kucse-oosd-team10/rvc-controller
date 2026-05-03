#include "rvc/i_cleaner.hpp"
#include "rvc/i_dust_sensor.hpp"
#include "rvc/i_motor.hpp"
#include "rvc/i_obstacle_sensor.hpp"
#include "rvc/types.hpp"

#include <memory>

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

// 초기화 후 IMotor::move가 전달받은 방향을 그대로 적용하는지 확인한다.
TEST(DeviceInterfacesTest, MotorAcceptsDirection) {
    TestMotor m;
    EXPECT_TRUE(m.initialize());
    EXPECT_TRUE(m.isInitialized);
    m.move(rvc::Direction::FORWARD);
    EXPECT_EQ(m.lastDirection, rvc::Direction::FORWARD);
    m.move(rvc::Direction::BACKWARD);
    EXPECT_EQ(m.lastDirection, rvc::Direction::BACKWARD);
}

// 갓 생성된 모터 스텁이 STOP 상태이며 아직 초기화되지 않았음을 확인한다.
TEST(DeviceInterfacesTest, MotorDefaultIsStop) {
    TestMotor m;
    EXPECT_EQ(m.lastDirection, rvc::Direction::STOP);
    EXPECT_FALSE(m.isInitialized);
}

// move()가 Direction enum의 모든 값을 IMotor 계약대로 처리하는지 확인한다.
TEST(DeviceInterfacesTest, MotorCoversAllDirections) {
    TestMotor m;
    for (auto d : {rvc::Direction::FORWARD, rvc::Direction::BACKWARD, rvc::Direction::LEFT,
                   rvc::Direction::RIGHT, rvc::Direction::STOP}) {
        m.move(d);
        EXPECT_EQ(m.lastDirection, d);
    }
}

// 연속된 move() 호출이 마지막 방향만 보존하도록 덮어쓰기 동작하는지 확인한다.
TEST(DeviceInterfacesTest, MotorOverwritesLastDirection) {
    TestMotor m;
    m.move(rvc::Direction::LEFT);
    m.move(rvc::Direction::RIGHT);
    EXPECT_EQ(m.lastDirection, rvc::Direction::RIGHT);
}

// IMotor 가상 디스패치: 베이스 포인터를 통한 호출이 구체 구현으로 도달하는지 확인한다.
TEST(DeviceInterfacesTest, MotorThroughInterfacePointer) {
    TestMotor m;
    rvc::IMotor* iface = &m;
    EXPECT_TRUE(iface->initialize());
    iface->move(rvc::Direction::FORWARD);
    EXPECT_EQ(m.lastDirection, rvc::Direction::FORWARD);
}

// std::unique_ptr<IMotor>를 통한 소멸로 IMotor의 가상 소멸자 경로가 실행되는지 확인한다.
TEST(DeviceInterfacesTest, MotorVirtualDestructorViaUniquePtr) {
    std::unique_ptr<rvc::IMotor> motor = std::make_unique<TestMotor>();
    motor->move(rvc::Direction::FORWARD);
    motor.reset();
    EXPECT_EQ(motor, nullptr);
}

// ICleaner::initialize가 구현체의 초기화 플래그를 세팅하고 true를 반환하는지 확인한다.
TEST(DeviceInterfacesTest, CleanerInitializeSetsFlag) {
    TestCleaner c;
    EXPECT_FALSE(c.isInitialized);
    EXPECT_TRUE(c.initialize());
    EXPECT_TRUE(c.isInitialized);
}

// 갓 생성된 클리너 스텁의 기본 전원 레벨이 OFF인지 확인한다.
TEST(DeviceInterfacesTest, CleanerDefaultPowerOff) {
    TestCleaner c;
    EXPECT_EQ(c.lastLevel, rvc::PowerLevel::OFF);
}

// setPower()가 PowerLevel enum의 모든 값을 ICleaner 계약대로 처리하는지 확인한다.
TEST(DeviceInterfacesTest, CleanerCoversAllPowerLevels) {
    TestCleaner c;
    for (auto level : {rvc::PowerLevel::OFF, rvc::PowerLevel::NORMAL, rvc::PowerLevel::POWER_UP}) {
        c.setPower(level);
        EXPECT_EQ(c.lastLevel, level);
    }
}

// ICleaner 가상 디스패치: 베이스 포인터를 통한 setPower 호출이 구현체에 도달하는지 확인한다.
TEST(DeviceInterfacesTest, CleanerThroughInterfacePointer) {
    TestCleaner c;
    rvc::ICleaner* iface = &c;
    iface->setPower(rvc::PowerLevel::POWER_UP);
    EXPECT_EQ(c.lastLevel, rvc::PowerLevel::POWER_UP);
}

// std::unique_ptr<ICleaner>를 통한 소멸로 ICleaner의 가상 소멸자 경로가 실행되는지 확인한다.
TEST(DeviceInterfacesTest, CleanerVirtualDestructorViaUniquePtr) {
    std::unique_ptr<rvc::ICleaner> cleaner = std::make_unique<TestCleaner>();
    cleaner->setPower(rvc::PowerLevel::NORMAL);
    cleaner.reset();
    EXPECT_EQ(cleaner, nullptr);
}

// 갓 생성된 장애물 센서가 모든 방향에서 미감지 상태로 보고하는지 확인한다.
TEST(DeviceInterfacesTest, ObstacleSensorDefaultsClear) {
    TestObstacleSensor s;
    EXPECT_TRUE(s.initialize());
    EXPECT_FALSE(s.isFrontDetected());
    EXPECT_FALSE(s.isLeftDetected());
    EXPECT_FALSE(s.isRightDetected());
}

// 전/좌/우 각 방향 플래그가 서로 독립적으로 보고되는지 확인한다.
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

// 세 방향 플래그를 동시에 활성화한 상황에서도 모두 감지로 보고됨을 확인한다.
TEST(DeviceInterfacesTest, ObstacleSensorAllDetected) {
    TestObstacleSensor s;
    s.front = s.left = s.right = true;
    EXPECT_TRUE(s.isFrontDetected());
    EXPECT_TRUE(s.isLeftDetected());
    EXPECT_TRUE(s.isRightDetected());
}

// IObstacleSensor 가상 디스패치: 베이스 포인터를 통한 방향별 질의가 구현체에 도달하는지 확인한다.
TEST(DeviceInterfacesTest, ObstacleSensorThroughInterfacePointer) {
    TestObstacleSensor s;
    rvc::IObstacleSensor* iface = &s;
    s.left = true;
    EXPECT_TRUE(iface->isLeftDetected());
    EXPECT_FALSE(iface->isFrontDetected());
}

// std::unique_ptr<IObstacleSensor>를 통한 소멸로 IObstacleSensor의 가상 소멸자 경로가 실행되는지 확인한다.
TEST(DeviceInterfacesTest, ObstacleSensorVirtualDestructorViaUniquePtr) {
    std::unique_ptr<rvc::IObstacleSensor> sensor = std::make_unique<TestObstacleSensor>();
    EXPECT_TRUE(sensor->initialize());
    sensor.reset();
    EXPECT_EQ(sensor, nullptr);
}

// IDustSensor::initialize가 스텁 구현에서 true를 반환하는지 확인한다.
TEST(DeviceInterfacesTest, DustSensorInitialize) {
    TestDustSensor d;
    EXPECT_TRUE(d.initialize());
}

// isDustDetected()가 내부 dust 플래그의 true/false 전환을 그대로 반영하는지 확인한다.
TEST(DeviceInterfacesTest, DustSensorReportsState) {
    TestDustSensor d;
    EXPECT_FALSE(d.isDustDetected());
    d.dust = true;
    EXPECT_TRUE(d.isDustDetected());
    d.dust = false;
    EXPECT_FALSE(d.isDustDetected());
}

// IDustSensor 가상 디스패치: 베이스 포인터를 통한 isDustDetected가 구현체에 도달하는지 확인한다.
TEST(DeviceInterfacesTest, DustSensorThroughInterfacePointer) {
    TestDustSensor d;
    rvc::IDustSensor* iface = &d;
    d.dust = true;
    EXPECT_TRUE(iface->isDustDetected());
}

// std::unique_ptr<IDustSensor>를 통한 소멸로 IDustSensor의 가상 소멸자 경로가 실행되는지 확인한다.
TEST(DeviceInterfacesTest, DustSensorVirtualDestructorViaUniquePtr) {
    std::unique_ptr<rvc::IDustSensor> sensor = std::make_unique<TestDustSensor>();
    EXPECT_FALSE(sensor->isDustDetected());
    sensor.reset();
    EXPECT_EQ(sensor, nullptr);
}

// initialize() 성공 후 setPower가 OFF가 아닌 레벨도 정상 반영하는지 확인한다.
TEST(DeviceInterfacesTest, CleanerAcceptsPowerLevel) {
    TestCleaner c;
    EXPECT_TRUE(c.initialize());
    c.setPower(rvc::PowerLevel::NORMAL);
    EXPECT_EQ(c.lastLevel, rvc::PowerLevel::NORMAL);
    c.setPower(rvc::PowerLevel::POWER_UP);
    EXPECT_EQ(c.lastLevel, rvc::PowerLevel::POWER_UP);
}

// 두 방향을 동시에 켠 복합 시나리오에서 각 축이 독립적으로 보고되는지 확인한다.
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

// dust 플래그가 기본 false에서 true로 토글될 때 isDustDetected()가 추종하는지 확인한다.
TEST(DeviceInterfacesTest, DustSensorReports) {
    TestDustSensor s;
    EXPECT_FALSE(s.isDustDetected());
    s.dust = true;
    EXPECT_TRUE(s.isDustDetected());
}

// 모터와 클리너 모두 베이스 포인터를 통해 올바르게 디스패치되는지 복합 검증한다.
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
