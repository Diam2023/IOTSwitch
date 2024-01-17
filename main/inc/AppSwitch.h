//
// Created by diam on 24-1-17.
//

#ifndef IOTSWITCH_APPSWITCH_H
#define IOTSWITCH_APPSWITCH_H

#include "Observer.h"
#include "esp_mqtt.hpp"
#include "esp_mqtt_client_config.hpp"
#include "Common.h"

class AppSwitch final : public LiveData<SwitchStatus> {
protected:

    void publishStatus();

public:

    void setWithoutPublish(const SwitchStatus &status);

    AppSwitch &operator=(const SwitchStatus &status) override;

    AppSwitch() : LiveData<SwitchStatus>(SwitchStatus::Close) {
    };

    static AppSwitch &getInstance() {
        static AppSwitch appSwitch;
        return appSwitch;
    }
};


#endif //IOTSWITCH_APPSWITCH_H
