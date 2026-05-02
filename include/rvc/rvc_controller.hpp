#pragma once

#include "rvc/i_rvc_state.hpp"
#include "rvc/i_sensor_observer.hpp"

namespace rvc {

class MovementManager;
class CleaningManager;
class ObstacleSensorSubject;
class DustSensorSubject;

class RVCController : public ISensorObserver {
public:
    RVCController();
    ~RVCController() override;

    RVCController(const RVCController&) = delete;
    RVCController& operator=(const RVCController&) = delete;
    RVCController(RVCController&&) = delete;
    RVCController& operator=(RVCController&&) = delete;

    void powerOn();
    void powerOff();

    void onObstacleDetected(bool front, bool left, bool right) override;
    void onDustDetected(bool detected) override;

    void setState(IRVCState* state);

    MovementManager* getMovementManager();
    CleaningManager* getCleaningManager();
    ObstacleSensorSubject* getObstacleSensorSubject();
    DustSensorSubject* getDustSensorSubject();

private:
    IRVCState* currentState_{nullptr};
    MovementManager* movementMgr_{nullptr};
    CleaningManager* cleaningMgr_{nullptr};
    ObstacleSensorSubject* obstacleSub_{nullptr};
    DustSensorSubject* dustSub_{nullptr};
};

} // namespace rvc
