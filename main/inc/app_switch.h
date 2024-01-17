//
// Created by diam on 24-1-17.
//

#ifndef IOTSWITCH_APP_SWITCH_H
#define IOTSWITCH_APP_SWITCH_H

#include "Observer.h"
#include "esp_mqtt.hpp"
#include "esp_mqtt_client_config.hpp"
#include "Common.h"

class AppSwitch : LiveData<SwitchStatus> {
private:
protected:
    void set(const SwitchStatus &status);

    void update(const SwitchStatus &status);

public:
    AppSwitch() {
        data = SwitchStatus::Close;
    };
};


#endif //IOTSWITCH_APP_SWITCH_H
