#pragma once

#include "rvc/i_rvc_state.hpp"
#include "rvc/i_sensor_observer.hpp"

namespace rvc {

class MovementManager;
class CleaningManager;
class ObstacleSensorSubject;
class DustSensorSubject;
class IObstacleSensor;

class RVCController : public ISensorObserver {
public:
    RVCController();
    ~RVCController() override;

    RVCController(const RVCController&) = delete;
    RVCController& operator=(const RVCController&) = delete;
    RVCController(RVCController&&) = delete;
    RVCController& operator=(RVCController&&) = delete;

    void powerOn() const;
    void powerOff();

    void onObstacleDetected(bool front, bool left, bool right) override;
    void onDustDetected(bool detected) override;

    void setState(IRVCState* state);
    [[nodiscard]] IRVCState* getCurrentState() const;

    MovementManager* getMovementManager();
    CleaningManager* getCleaningManager();
    ObstacleSensorSubject* getObstacleSensorSubject();
    DustSensorSubject* getDustSensorSubject();

    // AvoidingState 의 라이브 IObstacleSensor 조회 경로
    IObstacleSensor* getObstacleSensor();

    // TODO: 아래 nullable setter 3종은 Phase 1~2 의 임시 주입 API.
    // RVCController 생성자 주입으로 교체 후 제거 예정.
    void setMovementManager(MovementManager* manager);
    void setCleaningManager(CleaningManager* manager);
    void setObstacleSensor(IObstacleSensor* sensor);

private:
    IRVCState* currentState_{nullptr};
    MovementManager* movementMgr_{nullptr};
    CleaningManager* cleaningMgr_{nullptr};
    ObstacleSensorSubject* obstacleSub_{nullptr};
    DustSensorSubject* dustSub_{nullptr};
    IObstacleSensor* obstacleSensor_{nullptr};
};

} // namespace rvc
