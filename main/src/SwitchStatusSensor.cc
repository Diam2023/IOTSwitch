//
// Created by diam on 24-1-17.
//

#include "SwitchStatusSensor.h"

#include "VoltageSensor.h"
#include "AppConfig.h"
#include "AppSwitch.h"
#include "MqttClient.h"

SwitchStatusSensor::SwitchStatusSensor() : outputThreshold(AppConfig::getInstance().getOutputVoltageThreshold()) {
    VoltageSensor::getInstance();
}

void SwitchStatusSensor::onListener(const unsigned long &d) {
    if (d >= outputThreshold) {
        if (*AppSwitch::getInstance() == SwitchStatus::Close) {
            // Mute Update
            *AppSwitch::getInstance() = SwitchStatus::Open;
            // TODO Call Mqtt Send
//            MqttClientManager::getClient()->publishStatus();
        }
    } else {
        if (*AppSwitch::getInstance() == SwitchStatus::Open) {
            // Mute Update
            *AppSwitch::getInstance() = SwitchStatus::Close;
//            MqttClientManager::getClient()->publishStatus();
        }
    }
}
