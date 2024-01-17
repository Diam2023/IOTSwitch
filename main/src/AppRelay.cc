//
// Created by diam on 24-1-17.
//

#include "AppRelay.h"

using namespace idf;

AppRelay::AppRelay() : targetGpio(GPIONum(18)) {
    targetGpio.set_low();
}

void AppRelay::onListener(const SwitchStatus &d) {
    if (d == SwitchStatus::Open)
    {
        targetGpio.set_high();
    } else {
        targetGpio.set_low();
    }
}
