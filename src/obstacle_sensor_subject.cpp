#include "rvc/obstacle_sensor_subject.hpp"

#include <algorithm>

namespace rvc {

ObstacleSensorSubject::ObstacleSensorSubject(IObstacleSensor& sensor) : sensor_(sensor) {
}

void ObstacleSensorSubject::attach(ISensorObserver* obs) {
    if (obs == nullptr) {
        return;
    }
    if (std::ranges::find(observers_, obs) == observers_.end()) {
        observers_.push_back(obs);
    }
}

void ObstacleSensorSubject::detach(ISensorObserver* obs) {
    std::erase(observers_, obs);
}

void ObstacleSensorSubject::notify() {
    auto obsSnapshot = observers_;

    for (auto* obs : obsSnapshot) {
        if (obs != nullptr) {
            obs->onObstacleDetected(front_, left_, right_);
        }
    }
}

void ObstacleSensorSubject::poll() {
    const bool newFront = sensor_.isFrontDetected();
    const bool newLeft = sensor_.isLeftDetected();
    const bool newRight = sensor_.isRightDetected();

    if (newFront != front_ || newLeft != left_ || newRight != right_) {
        front_ = newFront;
        left_ = newLeft;
        right_ = newRight;
        notify();
    }
}

void ObstacleSensorSubject::onInterrupt() {
    front_ = sensor_.isFrontDetected();
    left_ = sensor_.isLeftDetected();
    right_ = sensor_.isRightDetected();
    notify();
}

} // namespace rvc
