//
// Created by diam on 24-1-17.
//

#ifndef IOTSWITCH_APPCONFIG_H
#define IOTSWITCH_APPCONFIG_H

#include <string>
#include "ArduinoJson/Document/StaticJsonDocument.hpp"


class AppConfig {
private:
    std::string configVersion;

    int64_t touchpadNotifyThreshold{};

    int64_t touchpadTouchLimitThreshold{};

    std::string mqttHost;
    std::string mqttPort;

    ArduinoJson::StaticJsonDocument<1024> configJson;

public:
    void load();

    void write();


    template<typename S>
    void loadJsonConfig(S &&);

    template<typename S>
    void writeJsonConfig(S &&);

    AppConfig() = default;

public:
    [[nodiscard]] const std::string &getConfigVersion() const {
        return configVersion;
    }

    void setConfigVersion(const std::string &configVersion_) {
        AppConfig::configVersion = configVersion_;
    }

    [[nodiscard]] int64_t getTouchpadNotifyThreshold() const {
        return touchpadNotifyThreshold;
    }

    void setTouchpadNotifyThreshold(int64_t touchpadNotifyThreshold_) {
        AppConfig::touchpadNotifyThreshold = touchpadNotifyThreshold_;
    }

    [[nodiscard]] int64_t getTouchpadTouchLimitThreshold() const {
        return touchpadTouchLimitThreshold;
    }

    void setTouchpadTouchLimitThreshold(int64_t touchpadTouchLimitThreshold_) {
        AppConfig::touchpadTouchLimitThreshold = touchpadTouchLimitThreshold_;
    }

    [[nodiscard]] const std::string &getMqttHost() const {
        return mqttHost;
    }

    void setMqttHost(const std::string &mqttHost_) {
        AppConfig::mqttHost = mqttHost_;
    }

    [[nodiscard]] const std::string &getMqttPort() const {
        return mqttPort;
    }

    void setMqttPort(const std::string &mqttPort_) {
        AppConfig::mqttPort = mqttPort_;
    }

};


#endif //IOTSWITCH_APPCONFIG_H
