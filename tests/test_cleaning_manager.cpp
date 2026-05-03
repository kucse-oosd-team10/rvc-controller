#include "rvc/cleaning_manager.hpp"
#include "rvc/dust_sensor_subject.hpp"
#include "rvc/i_cleaner.hpp"
#include "rvc/types.hpp"

#include <gtest/gtest.h>

namespace {

class MockCleaner : public rvc::ICleaner {
public:
    bool initialize() override {
        return true;
    }

    void setPower(rvc::PowerLevel level) override {
        lastPowerLevel = level;
        callCount++;
    }

    rvc::PowerLevel lastPowerLevel{rvc::PowerLevel::OFF};
    int callCount{0};
};

class MockClock {
public:
    std::int64_t now() const {
        return currentTime;
    }

    void advance(std::int64_t ms) {
        currentTime += ms;
    }

    std::int64_t currentTime{0};
};

} // namespace

class CleaningManagerTest : public ::testing::Test {
protected:
    MockCleaner cleaner;
    MockClock clock;
    // CleaningManager 인스턴스 초기화 (MockCleaner와 MockClock 사용)
    rvc::CleaningManager manager{&cleaner, nullptr, [this] {
                                     return clock.now();
                                 }};
};

/**
 * [상황] 청소를 시작할 때 (startCleaning 호출)
 * [기대 결과] 클리너의 파워 레벨이 NORMAL로 설정되어야 함.
 */
TEST_F(CleaningManagerTest, StartCleaningSetsNormalPower) {
    manager.startCleaning();
    EXPECT_EQ(cleaner.lastPowerLevel, rvc::PowerLevel::NORMAL);
    EXPECT_EQ(manager.getPowerLevel(), rvc::PowerLevel::NORMAL);
}

/**
 * [상황] 청소 도중 중지할 때 (stopCleaning 호출)
 * [기대 결과] 클리너의 파워 레벨이 OFF로 설정되어야 함.
 */
TEST_F(CleaningManagerTest, StopCleaningSetsPowerOff) {
    manager.startCleaning();
    manager.stopCleaning();
    EXPECT_EQ(cleaner.lastPowerLevel, rvc::PowerLevel::OFF);
    EXPECT_EQ(manager.getPowerLevel(), rvc::PowerLevel::OFF);
}

/**
 * [상황] 강제로 파워업 모드를 실행할 때 (powerUp 호출)
 * [기대 결과] 클리너의 파워 레벨이 POWER_UP으로 설정되어야 함.
 */
TEST_F(CleaningManagerTest, PowerUpSetsPowerUpLevel) {
    manager.powerUp();
    EXPECT_EQ(cleaner.lastPowerLevel, rvc::PowerLevel::POWER_UP);
    EXPECT_EQ(manager.getPowerLevel(), rvc::PowerLevel::POWER_UP);
}

/**
 * [상황] 먼지가 감지되었을 때 (handleDustDetected(true) 호출)
 * [기대 결과] 파워 레벨이 POWER_UP으로 변경되어야 함.
 */
TEST_F(CleaningManagerTest, HandleDustDetectedTriggersPowerUp) {
    manager.startCleaning();
    EXPECT_EQ(manager.getPowerLevel(), rvc::PowerLevel::NORMAL);

    manager.handleDustDetected(true);
    EXPECT_TRUE(manager.isDustDetected());
    EXPECT_EQ(cleaner.lastPowerLevel, rvc::PowerLevel::POWER_UP);
    EXPECT_EQ(manager.getPowerLevel(), rvc::PowerLevel::POWER_UP);
}

/**
 * [상황] 이미 POWER_UP 상태인데 먼지가 또 감지되었을 때
 * [기대 결과] 중복으로 setPower를 호출하지 않아야 함.
 */
TEST_F(CleaningManagerTest, HandleDustDetectedDoesNotPowerUpIfAlreadyPowerUp) {
    manager.powerUp();
    cleaner.callCount = 0; // 호출 횟수 초기화

    manager.handleDustDetected(true);
    EXPECT_EQ(cleaner.callCount, 0); // 추가 호출 없어야 함
}

/**
 * [상황] 먼지가 사라졌을 때 (handleDustDetected(false) 호출)
 * [기대 결과] 먼지 상태만 업데이트되고, 파워 레벨은 타이머 만료 전까지 POWER_UP 유지.
 */
TEST_F(CleaningManagerTest, HandleDustClearedJustUpdatesState) {
    manager.handleDustDetected(true);
    EXPECT_TRUE(manager.isDustDetected());

    manager.handleDustDetected(false);
    EXPECT_FALSE(manager.isDustDetected());
    EXPECT_EQ(manager.getPowerLevel(), rvc::PowerLevel::POWER_UP);
}

/**
 * [상황] 먼지가 없는 상태에서 POWER_UP 타이머가 만료되었을 때
 * [기대 결과] 파워 레벨이 NORMAL로 복구되어야 함.
 */
TEST_F(CleaningManagerTest, TimerExpiresAndReturnsToNormalIfNoDust) {
    manager.startCleaning();
    manager.handleDustDetected(true);
    manager.handleDustDetected(false); // 먼지 제거됨

    EXPECT_EQ(manager.getPowerLevel(), rvc::PowerLevel::POWER_UP);

    // 타이머 시간(3000ms)만큼 시간 진행
    clock.advance(rvc::CleaningManager::powerUpDuration + 1);
    manager.update();

    EXPECT_EQ(cleaner.lastPowerLevel, rvc::PowerLevel::NORMAL);
    EXPECT_EQ(manager.getPowerLevel(), rvc::PowerLevel::NORMAL);
}

/**
 * [상황] 먼지가 계속 감지되는 상태에서 POWER_UP 타이머가 만료되었을 때
 * [기대 결과] 타이머가 재설정되고 POWER_UP 상태가 유지되어야 함.
 */
TEST_F(CleaningManagerTest, TimerResetsIfDustStillDetected) {
    manager.startCleaning();
    manager.handleDustDetected(true); // 먼지 감지 유지

    // 타이머 만료 시간까지 진행
    clock.advance(rvc::CleaningManager::powerUpDuration + 1);
    manager.update();

    // 여전히 POWER_UP 상태여야 함
    EXPECT_EQ(manager.getPowerLevel(), rvc::PowerLevel::POWER_UP);

    // 먼지 제거 후 다시 시간 진행
    manager.handleDustDetected(false);
    clock.advance(rvc::CleaningManager::powerUpDuration + 1);
    manager.update();

    EXPECT_EQ(manager.getPowerLevel(), rvc::PowerLevel::NORMAL);
}

/**
 * [상황] POWER_UP 도중 청소를 중지했을 때
 * [기대 결과] 타이머가 멈추고 파워는 OFF 상태여야 함.
 */
TEST_F(CleaningManagerTest, StopCleaningStopsTimer) {
    manager.powerUp();
    manager.stopCleaning();

    clock.advance(rvc::CleaningManager::powerUpDuration + 1);
    manager.update();

    // 타이머에 의해 NORMAL로 돌아가지 않고 OFF 상태 유지
    EXPECT_EQ(manager.getPowerLevel(), rvc::PowerLevel::OFF);
}

/**
 * [상황] 클리너 객체가 Null인 경우 각 메서드 호출
 * [기대 결과] 크래시 없이 안전하게 리턴되어야 함 (방어적 코드 테스트)
 */
TEST(CleaningManagerNullTest, SafeWithNullCleaner) {
    MockClock clock;
    rvc::CleaningManager nullManager{nullptr, nullptr, [&] {
                                         return clock.now();
                                     }};

    // 크래시 여부 확인을 위한 호출들
    EXPECT_NO_THROW(nullManager.startCleaning());
    EXPECT_NO_THROW(nullManager.stopCleaning());
    EXPECT_NO_THROW(nullManager.powerUp());

    // Timer expired case with null cleaner
    nullManager.powerUp();
    clock.advance(rvc::CleaningManager::powerUpDuration + 1);
    EXPECT_NO_THROW(nullManager.update());
}

/**
 * [상황] 먼지 상태 조회
 * [기대 결과] 설정된 먼지 상태를 정확히 반환해야 함.
 */
TEST_F(CleaningManagerTest, IsDustDetectedReportsCorrectState) {
    manager.handleDustDetected(true);
    EXPECT_TRUE(manager.isDustDetected());
    manager.handleDustDetected(false);
    EXPECT_FALSE(manager.isDustDetected());
}
