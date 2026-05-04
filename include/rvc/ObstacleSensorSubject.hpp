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
    explicit ObstacleSensorSubject(std::unique_ptr<IObstacleSensor> frontSensor,
                                   std::unique_ptr<IObstacleSensor> leftSensor,
                                   std::unique_ptr<IObstacleSensor> rightSensor);
    ObstacleSensorSubject(const ObstacleSensorSubject&) = delete;
    ObstacleSensorSubject& operator=(const ObstacleSensorSubject&) = delete;
    ObstacleSensorSubject(ObstacleSensorSubject&&) = delete;
    ObstacleSensorSubject& operator=(ObstacleSensorSubject&&) = delete;
    virtual ~ObstacleSensorSubject() = default;

    void attach(ISensorObserver* observer) override;
    void detach(ISensorObserver* observer) override;
    void notify() override;
    void poll();
    void onInterrupt();

private:
    std::unique_ptr<IObstacleSensor> frontSensor_ = nullptr;
    std::unique_ptr<IObstacleSensor> leftSensor_ = nullptr;
    std::unique_ptr<IObstacleSensor> rightSensor_ = nullptr;
    bool frontDetected_{false};
    bool leftDetected_{false};
    bool rightDetected_{false};
    std::list<ISensorObserver*> observers_;
    Timer pollingTimer_;
};

} // namespace rvc
