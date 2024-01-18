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
    uint32_t mqttPort = 1883;

    bool beepMute = false;

    /**
     * Output Voltage switch threshold
     */
    uint32_t outputVoltageThreshold = 50;

    /**
     * Temperature to warning
     */
    uint32_t temperatureVoltageThreshold = 2700;

    /**
     * SSID For Wifi
     */
    std::string wifiSsid;

    /**
     * PWD For Wifi
     */
    std::string wifiPwd;
private:

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
    [[nodiscard]] const std::string &getWifiSsid() const {
        return wifiSsid;
    }

    void setWifiSsid(const std::string &wifiSsid_) {
        AppConfig::wifiSsid = wifiSsid_;
    }

    [[nodiscard]] const std::string &getWifiPwd() const {
        return wifiPwd;
    }

    void setWifiPwd(const std::string &wifiPwd_) {
        AppConfig::wifiPwd = wifiPwd_;
    }

    [[nodiscard]] uint32_t getOutputVoltageThreshold() const {
        return outputVoltageThreshold;
    }

    void setOutputVoltageThreshold(uint32_t outputVoltageThreshold_) {
        AppConfig::outputVoltageThreshold = outputVoltageThreshold_;
    }

    [[nodiscard]] uint32_t getTemperatureVoltageThreshold() const {
        return temperatureVoltageThreshold;
    }

    void setTemperatureVoltageThreshold(uint32_t outputVoltageThreshold_) {
        AppConfig::temperatureVoltageThreshold = outputVoltageThreshold_;
    }

    [[nodiscard]] bool getBeepMute() const {
        return beepMute;
    }

    void setBeepMute(bool beepMute_) {
        AppConfig::beepMute = beepMute_;
    }

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
