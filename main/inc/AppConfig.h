//
// Created by diam on 24-1-17.
//

#ifndef IOTSWITCH_APPCONFIG_H
#define IOTSWITCH_APPCONFIG_H

#include <string>
#include <ArduinoJson.hpp>


class AppConfig {
private:
    std::string configVersion;

    int64_t touchpadNotifyThreshold{};

    int64_t touchpadTouchLimitThreshold{};

    std::string mqttHost;
    uint32_t mqttPort;

    ArduinoJson::StaticJsonDocument<1024> configJson;

    std::string deviceSerialNumber;

public:
    AppConfig &load();

    AppConfig &write();

    template<typename S>
    void loadJsonConfig(S &&);

    template<typename S>
    void writeJsonConfig(S &&);

    AppConfig() = default;

    static AppConfig &getInstance() {
        static AppConfig config;
        return config;
    }

public:
    [[nodiscard]] const std::string &getDeviceSerialNumber() const {
        return deviceSerialNumber;
    }

    void setDeviceSerialNumber(const std::string &serialNumber_) {
        AppConfig::deviceSerialNumber = serialNumber_;
    }

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

    [[nodiscard]] const uint32_t &getMqttPort() const {
        return mqttPort;
    }

    void setMqttPort(uint32_t mqttPort_) {
        AppConfig::mqttPort = mqttPort_;
    }

};


#endif //IOTSWITCH_APPCONFIG_H
