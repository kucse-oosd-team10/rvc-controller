#pragma once

#include "rvc/i_obstacle_sensor.hpp"
#include "rvc/i_sensor_observer.hpp"

#include <vector>

namespace rvc {

class ObstacleSensorSubject : public ISensorSubject {
public:
    explicit ObstacleSensorSubject(IObstacleSensor& sensor);
    ~ObstacleSensorSubject() override = default;

    ObstacleSensorSubject(const ObstacleSensorSubject&) = delete;
    ObstacleSensorSubject& operator=(const ObstacleSensorSubject&) = delete;
    ObstacleSensorSubject(ObstacleSensorSubject&&) = delete;
    ObstacleSensorSubject& operator=(ObstacleSensorSubject&&) = delete;

    void attach(ISensorObserver* obs) override;
    void detach(ISensorObserver* obs) override;
    void notify() override;

    void poll();
    void onInterrupt();

private:
    IObstacleSensor& sensor_;
    std::vector<ISensorObserver*> observers_;
    bool front_{false};
    bool left_{false};
    bool right_{false};
};

} // namespace rvc
