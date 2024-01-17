#pragma once

#include "esp_afe_sr_iface.h"
#include "esp_process_sdkconfig.h"
#include "model_path.h"

#include "Observer.h"

#include "Common.h"


// TODO Restructure
class SpeechSensor {
public:
    const esp_afe_sr_iface_t *afe_handle;
    esp_afe_sr_data_t *afe_data;
    srmodel_list_t *models;

    LiveData<SpeechSensorStatus> speechSensorStatus;

    LiveData<SwitchStatus> switchCommandStatus;

    SpeechSensor();

    static SpeechSensor &getInstance() {
        static SpeechSensor speechSensor;
        return speechSensor;
    };

    void run();
};
