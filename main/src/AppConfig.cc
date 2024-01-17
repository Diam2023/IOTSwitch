//
// Created by diam on 24-1-17.
//
#include "AppConfig.h"

#include "esp_log.h"
#include <fstream>
#include <sstream>
#include "Common.h"
#include <ArduinoJson.hpp>

static const char *TAG = "AppConfig";

void AppConfig::load() {
    try {
        ESP_LOGD(TAG, "Load config from file");
        std::ifstream ifs(g_pConfigFilePath, std::ifstream::in);
        std::stringstream buffer;
        buffer << ifs.rdbuf();
        std::string dt(buffer.str());
        loadJsonConfig(dt);
    }
    catch (const std::exception &e) {
        ESP_LOGE(TAG, "Error For Decode JSON: %s", e.what());
    }
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
}

void AppConfig::write() {

    try {

        std::ofstream ofs(g_pConfigFilePath, std::ofstream::out);
        writeJsonConfig(ofs);
        ESP_LOGD(TAG, "write successful!");
    } catch (const std::exception &e) {
        ESP_LOGD(TAG, "Error for write Config file: %s", e.what());
    }
}

template<typename S>
void AppConfig::writeJsonConfig(S &&stream) {

    configJson.clear();

    configJson["configVersion"] = this->getConfigVersion();
    configJson["touchpad"]["notifyThreshold"] = this->getTouchpadNotifyThreshold();
    configJson["touchpad"]["touchLimitThreshold"] = this->getTouchpadTouchLimitThreshold();
    configJson["mqtt"]["host"] = this->getMqttHost();
    configJson["mqtt"]["port"] = this->getMqttPort();

    // encode This Json
    ArduinoJson::serializeJson(configJson, stream);
}
