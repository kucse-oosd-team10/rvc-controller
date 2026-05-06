#include "rvc/obstacle_sensor_subject.hpp"

#include <algorithm>

namespace rvc {

ObstacleSensorSubject::ObstacleSensorSubject(std::unique_ptr<IObstacleSensor> sensor)
    : sensor_{std::move(sensor)} {
    pollingTimer_.setDuration(100); // Poll every 100 ms
    pollingTimer_.start();
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
    if (!sensor_) {
        return;
    }

    for (auto* obs : observers_) {
        obs->onObstacleDetected(currentFrontDetected_, currentLeftDetected_, currentRightDetected_);
    }
}

void ObstacleSensorSubject::poll() {
    if (!sensor_ || !pollingTimer_.isExpired()) {
        return;
    }
    //아래 코드를 원래 poll()에 구현했다가 updateObstacleState()로 분리했는데 괜찮을 지 의견
    //부탁드립니다.
    // const bool currentFront = sensor_->isFrontDetected();
    // const bool currentLeft = sensor_->isLeftDetected();
    // const bool currentRight = sensor_->isRightDetected();
    // const bool changed = !hasCurrentState_ || currentFront != currentFrontDetected_ ||
    //                      currentLeft != currentLeftDetected_ || currentRight !=
    //                      currentRightDetected_;

    // currentFrontDetected_ = currentFront;
    // currentLeftDetected_ = currentLeft;
    // currentRightDetected_ = currentRight;
    // hasCurrentState_ = true;

    pollingTimer_.reset();

    if (updateObstacleState()) {
        notify();
    }
}

// cppcheck-suppress unusedFunction
void ObstacleSensorSubject::onInterrupt() {
    if (!sensor_) {
        return;
    }
    //역시 updateObstacleState로 분리하여 센서값 읽어온 후 notify() 호출하도록 구현했습니다.
    if (updateObstacleState()) {
        notify();
    }
}

bool ObstacleSensorSubject::updateObstacleState() {
    if (!sensor_) {
        return false;
    }

    const bool currentFront = sensor_->isFrontDetected();
    const bool currentLeft = sensor_->isLeftDetected();
    const bool currentRight = sensor_->isRightDetected();

    const bool changed = !hasCurrentState_ || currentFront != currentFrontDetected_ ||
                         currentLeft != currentLeftDetected_ ||
                         currentRight != currentRightDetected_;

    currentFrontDetected_ = currentFront;
    currentLeftDetected_ = currentLeft;
    currentRightDetected_ = currentRight;
    hasCurrentState_ = true;

    return changed;
}

} // namespace rvc