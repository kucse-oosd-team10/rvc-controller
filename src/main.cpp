#include "rvc/controller.hpp"

#include <iostream>

int main() {
    rvc::Controller ctrl;
    ctrl.setFrontSensor(true);
    ctrl.tick();

    std::cout << "RVC Controller initialized.\n";
    std::cout << "Stopped: " << (ctrl.isStopped() ? "yes" : "no") << "\n";
    return 0;
}