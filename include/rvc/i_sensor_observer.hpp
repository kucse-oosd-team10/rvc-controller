#pragma once

namespace rvc {

class ISensorObserver {
public:
    ISensorObserver() = default;
    virtual ~ISensorObserver() = default;

    ISensorObserver(const ISensorObserver&) = delete;
    ISensorObserver& operator=(const ISensorObserver&) = delete;
    ISensorObserver(ISensorObserver&&) = delete;
    ISensorObserver& operator=(ISensorObserver&&) = delete;

    virtual void onObstacleDetected(bool front, bool left, bool right) = 0;
    virtual void onDustDetected(bool detected) = 0;
};

class ISensorSubject {
public:
    ISensorSubject() = default;
    virtual ~ISensorSubject() = default;

    ISensorSubject(const ISensorSubject&) = delete;
    ISensorSubject& operator=(const ISensorSubject&) = delete;
    ISensorSubject(ISensorSubject&&) = delete;
    ISensorSubject& operator=(ISensorSubject&&) = delete;

    virtual void attach(ISensorObserver* obs) = 0;
    virtual void detach(ISensorObserver* obs) = 0;
    virtual void notify() = 0;
};

} // namespace rvc
