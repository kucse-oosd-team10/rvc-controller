#pragma once

#include "rvc/i_dust_sensor.hpp"
#include "rvc/i_sensor_observer.hpp"

#include <vector>

namespace rvc {

class DustSensorSubject : public ISensorSubject {
public:
    explicit DustSensorSubject(IDustSensor& sensor);
    ~DustSensorSubject() override = default;

    DustSensorSubject(const DustSensorSubject&) = delete;
    DustSensorSubject& operator=(const DustSensorSubject&) = delete;
    DustSensorSubject(DustSensorSubject&&) = delete;
    DustSensorSubject& operator=(DustSensorSubject&&) = delete;

    void attach(ISensorObserver* obs) override;
    void detach(ISensorObserver* obs) override;
    void notify() override;
    void poll();
    bool isDustDetected() const;

private:
    IDustSensor& sensor_;
    std::vector<ISensorObserver*> observers_;
    bool dustDetected_{false};
};

} // namespace rvc