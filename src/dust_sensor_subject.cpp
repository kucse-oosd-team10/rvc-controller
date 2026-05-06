#include "rvc/dust_sensor_subject.hpp"

#include <algorithm>

namespace rvc {

DustSensorSubject::DustSensorSubject(IDustSensor* sensor) : sensor_(sensor) {
}

void DustSensorSubject::attach(ISensorObserver* obs) {
    if (obs == nullptr) {
        return;
    }
    if (std::ranges::find(observers_, obs) == observers_.end()) {
        observers_.push_back(obs);
    }
}

void DustSensorSubject::detach(ISensorObserver* obs) {
    std::erase(observers_, obs);
}

void DustSensorSubject::notify() {
    auto obsSnapshot = observers_;

    for (auto* obs : obsSnapshot) {
        if (obs != nullptr) {
            obs->onDustDetected(dustDetected_);
        }
    }
}

void DustSensorSubject::poll() {
    if (sensor_ == nullptr) {
        return;
    }

    const bool newDust = sensor_->isDustDetected();
    if (dustDetected_ != newDust) {
        dustDetected_ = newDust;
        notify();
    }
}

bool DustSensorSubject::isDustDetected() const {
    return dustDetected_;
}

} // namespace rvc