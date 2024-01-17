//
// Created by diam on 24-1-17.
//
#include "AppConfig.h"

#include "esp_log.h"
#include <fstream>
#include <sstream>
#include "Common.h"
#include <ArduinoJson.hpp>

#include <esp_mac.h>
#include <iostream>

static const char *TAG = "AppConfig";

bool readFactorySetMacAddress(uint8_t *macAddress) {
    return (esp_efuse_mac_get_default(macAddress) == ESP_OK);
}

AppConfig &AppConfig::load() {
    try {
        ESP_LOGD(TAG, "Load config from file");
        std::ifstream ifs(g_pConfigFilePath, std::ifstream::in);
        std::stringstream buffer;
        buffer << ifs.rdbuf();
        std::cout << ifs.rdbuf() << std::endl;
        std::string dt(buffer.str());
        loadJsonConfig(dt);
    }
    catch (const std::exception &e) {
        ESP_LOGE(TAG, "Error For Decode JSON: %s", e.what());
    }

    return *this;
}

template<typename S>
void AppConfig::loadJsonConfig(S &&stream) {
    configJson.clear();
    auto error = ArduinoJson::deserializeJson(configJson, stream);

    if (error) {
        // Throw A Error For Json Read
        esp_log_write(ESP_LOG_ERROR, TAG, "%s", error.c_str());
    }

    if (!configJson["configVersion"].isNull()) {
        this->setConfigVersion(configJson["configVersion"]);
    }
    if (!configJson["touchpad"]["notifyThreshold"].isNull()) {
        this->setTouchpadNotifyThreshold(configJson["touchpad"]["notifyThreshold"]);
    }
    if (!configJson["touchpad"]["touchLimitThreshold"].isNull()) {
        this->setTouchpadTouchLimitThreshold(configJson["touchpad"]["touchLimitThreshold"]);
    }
    if (!configJson["mqtt"]["host"].isNull()) {
        this->setMqttHost(configJson["mqtt"]["host"]);
    }
    if (!configJson["mqtt"]["port"].isNull()) {
        this->setMqttPort(configJson["mqtt"]["port"]);
    }

    if (configJson["deviceSerialNumber"].isNull()) {

    }

    std::string SN = configJson["deviceSerialNumber"];
    if (SN.length() < 16) {
        uint8_t macAddress[8];
        char macAddressString[17];
        if (readFactorySetMacAddress(macAddress)) {
            sprintf(macAddressString, "%02X%02X%02X%02X%02X%02X%02X%02X", (uint16_t) macAddress[0],
                    (uint16_t) macAddress[1], (uint16_t) macAddress[2], (uint16_t) macAddress[3],
                    (uint16_t) macAddress[4],
                    (uint16_t) macAddress[5], (uint16_t) macAddress[6], (uint16_t) macAddress[7]);
            this->setDeviceSerialNumber(std::string(macAddressString));
        } else {
            ESP_LOGE(TAG, "FAULT ERROR: SN Not Found!");
        }
    }
}

AppConfig &AppConfig::write() {

    try {

        std::ofstream ofs(g_pConfigFilePath, std::ofstream::out);
        writeJsonConfig(ofs);
        ESP_LOGD(TAG, "write successful!");
    } catch (const std::exception &e) {
        ESP_LOGD(TAG, "Error for write Config file: %s", e.what());
    }

    return *this;
}

template<typename S>
void AppConfig::writeJsonConfig(S &&stream) {

    configJson.clear();

    configJson["configVersion"] = this->getConfigVersion();
    configJson["touchpad"]["notifyThreshold"] = this->getTouchpadNotifyThreshold();
    configJson["touchpad"]["touchLimitThreshold"] = this->getTouchpadTouchLimitThreshold();
    configJson["mqtt"]["host"] = this->getMqttHost();
    configJson["mqtt"]["port"] = this->getMqttPort();

    // Ignore SN

    // encode This Json
    ArduinoJson::serializeJson(configJson, stream);
}
