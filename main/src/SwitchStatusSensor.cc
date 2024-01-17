//
// Created by diam on 24-1-17.
//

#include "SwitchStatusSensor.h"

#include "VoltageSensor.h"

SwitchStatusSensor::SwitchStatusSensor() {
    VoltageSensor::getInstance();
}
