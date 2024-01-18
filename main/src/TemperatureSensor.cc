//
// Created by diam on 24-1-17.
//

#include "TemperatureSensor.h"

#include "AppConfig.h"
#include "AppBuzzer.h"
#include "MqttClient.h"

#include <esp_log.h>
#include <string>
#include <sstream>

static const char *TAG = "TEMPERATURE";

// TODO prepare a mutex
void TemperatureSensor::onListener(const unsigned long &d) {
    if (d >= temperatureThreshold) {
        static int counter = 0;
        if (counter++ >= 5) {
            AppBuzzer::getInstance().beepMicroTimeSync({5, 30});
            static std::stringstream ss;
            ss.clear();
            ss << "TEMPERATURE HEIGHT: " << d << "mV";
            std::string msg = ss.str();
            MqttClientManager::getClient()->publishWarning(msg);
            ESP_LOGE(TAG, "TEMPERATURE HEIGHT!!! %lu mV", d);
            counter = 0;
        }
    }
}

TemperatureSensor::TemperatureSensor() :
        temperatureThreshold(
                AppConfig::getInstance().getTemperatureVoltageThreshold()) {
}
