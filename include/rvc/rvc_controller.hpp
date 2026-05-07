#pragma once

#include "rvc/cleaning_state.hpp"
#include "rvc/error_state.hpp"
#include "rvc/i_rvc_state.hpp"
#include "rvc/i_sensor_observer.hpp"
#include "rvc/initializing_state.hpp"
#include "rvc/off_state.hpp"

#include <memory>

namespace rvc {

class MovementManager;
class CleaningManager;
class ObstacleSensorSubject;
class DustSensorSubject;
class IObstacleSensor;
class IDustSensor;
class IMotor;
class ICleaner;
class AvoidingState;

class RVCController : public ISensorObserver {
public:
    RVCController(IObstacleSensor& obstacleSensor, IDustSensor& dustSensor, IMotor& motor,
                  ICleaner& cleaner, MovementManager& movementMgr, CleaningManager& cleaningMgr,
                  ObstacleSensorSubject& obstacleSub, DustSensorSubject& dustSub);
    ~RVCController() override;

    RVCController(const RVCController&) = delete;
    RVCController& operator=(const RVCController&) = delete;
    RVCController(RVCController&&) = delete;
    RVCController& operator=(RVCController&&) = delete;

    void powerOn();
    void powerOff();
    void tick();

    void onObstacleDetected(bool front, bool left, bool right) override;
    void onDustDetected(bool detected) override;

    void setState(IRVCState* state);
    [[nodiscard]] IRVCState* getCurrentState() const;

    void enterOff();
    void enterInitializing();
    void enterCleaning();
    void enterError();
    void enterAvoiding(bool front, bool left, bool right);

    MovementManager* getMovementManager();
    CleaningManager* getCleaningManager();
    ObstacleSensorSubject* getObstacleSensorSubject();
    DustSensorSubject* getDustSensorSubject();
    IObstacleSensor* getObstacleSensor();

private:
    IObstacleSensor* obstacleSensor_;
    IDustSensor* dustSensor_;
    IMotor* motor_;
    ICleaner* cleaner_;
    MovementManager* movementMgr_;
    CleaningManager* cleaningMgr_;
    ObstacleSensorSubject* obstacleSub_;
    DustSensorSubject* dustSub_;

    OffState offState_;
    InitializingState initializingState_;
    CleaningState cleaningState_;
    ErrorState errorState_;
    std::unique_ptr<AvoidingState> currentAvoiding_;

    IRVCState* currentState_{nullptr};
};

} // namespace rvc
