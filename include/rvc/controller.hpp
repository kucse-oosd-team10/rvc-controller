#pragma once

namespace rvc {

enum class Direction {
    Forward,
    Backward,
    Left,
    Right
};
enum class CleanMode {
    Off,
    On,
    PowerUp
};

class Controller {
public:
    Controller();

    void tick();

    Direction getDirection() const;
    CleanMode getCleanMode() const;

    void setFrontSensor(bool detected);
    void setLeftSensor(bool detected);
    void setRightSensor(bool detected);
    void setDustSensor(bool detected);

private:
    bool frontSensor_{false};
    bool leftSensor_{false};
    bool rightSensor_{false};
    bool dustSensor_{false};

    Direction direction_{Direction::Forward};
    CleanMode cleanMode_{CleanMode::On};
};

} // namespace rvc