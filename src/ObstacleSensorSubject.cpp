#include "rvc/ObstacleSensorSubject.hpp"

#include <algorithm>

namespace rvc {

ObstacleSensorSubject::ObstacleSensorSubject(std::unique_ptr<IObstacleSensor> frontSensor,
                                             std::unique_ptr<IObstacleSensor> leftSensor,
                                             std::unique_ptr<IObstacleSensor> rightSensor)
    : frontSensor_{std::move(frontSensor)}, leftSensor_{std::move(leftSensor)},
      rightSensor_{std::move(rightSensor)} {
    if (leftSensor_ || rightSensor_) {
        pollingTimer_.setDuration(100); // Poll every 100 ms
        pollingTimer_.start();
    }
}

void ObstacleSensorSubject::attach(ISensorObserver* obs) {
    if (obs == nullptr) {
        return;
    }

    if (std::find(observers_.begin(), observers_.end(), obs) != observers_.end()) {
        return;
    }

    observers_.push_back(obs);
}

void ObstacleSensorSubject::detach(ISensorObserver* obs) {
    if (obs == nullptr) {
        return;
    }

    observers_.remove(obs);
}

void ObstacleSensorSubject::notify() {
    if (!frontSensor_ && !leftSensor_ && !rightSensor_) {
        return;
    }

    if (frontSensor_) {
        frontDetected_ = frontSensor_->isFrontDetected();
    }
    if (leftSensor_) {
        leftDetected_ = leftSensor_->isLeftDetected();
    }
    if (rightSensor_) {
        rightDetected_ = rightSensor_->isRightDetected();
    }

    for (auto* obs : observers_) {
        obs->onObstacleDetected(frontDetected_, leftDetected_, rightDetected_);
    }
}

void ObstacleSensorSubject::poll() {
    if (pollingTimer_.isExpired()) {
        pollingTimer_.reset();
        if (leftSensor_) {
            leftDetected_ = leftSensor_->isLeftDetected();
        }
        if (rightSensor_) {
            rightDetected_ = rightSensor_->isRightDetected();
        }

        for (auto* obs : observers_) {
            obs->onObstacleDetected(frontDetected_, leftDetected_, rightDetected_);
        }
    }
}

// cppcheck-suppress unusedFunction
void ObstacleSensorSubject::onInterrupt() {
    if (!frontSensor_) {
        return;
    }

    frontDetected_ = frontSensor_->isFrontDetected();

    for (auto* obs : observers_) {
        obs->onObstacleDetected(frontDetected_, leftDetected_, rightDetected_);
    }
}

} // namespace rvc