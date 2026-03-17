#include "rvc/controller.hpp"

#include <iostream>

int main() {
    rvc::Controller ctrl;
    ctrl.setFrontSensor(true);
    ctrl.tick();

    std::cout << "RVC Controller initialized.\n";
    return 0;
}