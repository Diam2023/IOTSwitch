//
// Created by diam on 24-1-17.
//

#include "SwitchStatusSensor.h"

#include "VoltageSensor.h"
#include "AppConfig.h"
#include "AppSwitch.h"

SwitchStatusSensor::SwitchStatusSensor() : outputThreshold(AppConfig::getInstance().getOutputVoltageThreshold()) {
    VoltageSensor::getInstance();
}

void SwitchStatusSensor::onListener(const unsigned long &d) {
    if (d >= outputThreshold) {
        if (*AppSwitch::getInstance() == SwitchStatus::Close) {
            // Update
            AppSwitch::getInstance() = SwitchStatus::Open;
        }
    } else {
        if (*AppSwitch::getInstance() == SwitchStatus::Open) {
            // Update
            AppSwitch::getInstance() = SwitchStatus::Close;
        }
    }
}
