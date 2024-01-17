//
// Created by diam on 24-1-17.
//

#include "TemperatureSensor.h"

#include "AppConfig.h"
#include "AppBuzzer.h"

#include <esp_log.h>

static const char *TAG = "TEMPERATURE";

// TODO prepare a mutex
void TemperatureSensor::onListener(const unsigned long &d) {

    if (d >= temperatureThreshold) {
        AppBuzzer::getInstance().beepMicroTimeSync({5, 50});
        ESP_LOGE(TAG, "TEMPERATURE HEIGHT!!! %lu", d);
    }
}

TemperatureSensor::TemperatureSensor() :
        temperatureThreshold(
                AppConfig::getInstance().getTemperatureVoltageThreshold()) {
}
