#pragma once

#include "rvc/i_obstacle_sensor.hpp"
#include "rvc/i_sensor_observer.hpp"

#include <functional>
#include <list>
#include <memory>

#include "timer.hpp"

namespace rvc {

class ObstacleSensorSubject : public ISensorSubject {
public:
    ObstacleSensorSubject() = default;
    explicit ObstacleSensorSubject(std::unique_ptr<IObstacleSensor> sensor);
    ObstacleSensorSubject(const ObstacleSensorSubject&) = delete;
    ObstacleSensorSubject& operator=(const ObstacleSensorSubject&) = delete;
    ObstacleSensorSubject(ObstacleSensorSubject&&) = delete;
    ObstacleSensorSubject& operator=(ObstacleSensorSubject&&) = delete;
    ~ObstacleSensorSubject() override = default;

    void attach(ISensorObserver* observer) override;
    void detach(ISensorObserver* observer) override;
    void notify() override;
    void poll();
    void onInterrupt();

private:
    std::unique_ptr<IObstacleSensor> sensor_ = nullptr;
    bool useInterrupt_{false};

    // poll()에서 이전 상태와 비교하기 위한 변수들
    bool hasCurrentState_{false};
    bool currentFrontDetected_{false};
    bool currentLeftDetected_{false};
    bool currentRightDetected_{false};
    std::list<ISensorObserver*> observers_;
    Timer pollingTimer_;

    bool updateObstacleState();
};

} // namespace rvc
