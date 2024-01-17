//
// Created by diam on 24-1-17.
//

#include "AppSwitch.h"

#include "MqttClient.h"

AppSwitch &AppSwitch::operator=(const SwitchStatus &status) {
    LiveData<SwitchStatus>::operator=(status);

    // call publish
    MqttClientManager::getClient()->publishStatus();
    return *this;
}

void AppSwitch::setWithoutPublish(const SwitchStatus &status) {
    LiveData<SwitchStatus>::operator=(status);
}
