//
// Created by diam on 24-1-17.
//

#include "MqttClient.h"
#include "AppSwitch.h"

#include <sstream>
#include <string>
#include <utility>
#include <esp_log.h>

static const char *TAG = "MqttClient";

MqttClient::MqttClient(const idf::mqtt::BrokerConfiguration &broker, const idf::mqtt::ClientCredentials &credentials,
                       const idf::mqtt::Configuration &config) : idf::mqtt::Client(broker, credentials, config),
                                                                 connectionStatusLiveData(MqttConnectionStatus::Init) {
    std::stringstream dataFilterString;
    std::stringstream configFilterString;

    dataFilterString << "$SYS/devices/" << AppConfig::getInstance().getDeviceSerialNumber() << "/data";
    configFilterString << "$SYS/devices/" << AppConfig::getInstance().getDeviceSerialNumber() << "/config";

    dataFilterPtr = std::make_unique<idf::mqtt::Filter>(dataFilterString.str());
    configFilterPtr = std::make_unique<idf::mqtt::Filter>(configFilterString.str());

    ESP_LOGI(TAG, "Subscribe Data Filter: %s", dataFilterPtr->get().c_str());
    ESP_LOGI(TAG, "Subscribe Config Filter: %s", configFilterPtr->get().c_str());
};

void MqttClient::on_connected(esp_mqtt_event_handle_t event) {
    subscribe(dataFilterPtr->get(), idf::mqtt::QoS::AtMostOnce);
    subscribe(configFilterPtr->get(), idf::mqtt::QoS::AtMostOnce);
    connectionStatusLiveData = MqttConnectionStatus::Connected;
}

void MqttClient::on_data(esp_mqtt_event_handle_t event) {
    std::string jsonConfig(event->data, event->data_len);
    if (jsonConfig.empty()) {
        ESP_LOGW(TAG, "Ignore MqttClient Config Empty Data");
        return;
    }
    std::stringstream ss;
    ss << jsonConfig.c_str();
    if (configFilterPtr->match(event->topic, event->topic_len)) {
        AppConfig::getInstance().writeJsonConfig(ss);
    } else if (dataFilterPtr->match(event->topic, event->topic_len)) {
        ArduinoJson::DynamicJsonDocument dynamicJsonDocument(1024);
        auto error = ArduinoJson::deserializeJson(dynamicJsonDocument, ss);
        if (error) {
            ESP_LOGW(TAG, "Deserialize Json Data Error");
            return;
        }
        auto switchJson = dynamicJsonDocument["data"];
        if (!switchJson.isNull()) {
            auto setStatus = ((bool) switchJson) ? SwitchStatus::Open : SwitchStatus::Close;
            // Skip Callback loop!
            AppSwitch::getInstance().setWithoutPublish(setStatus);
        } else {
            ESP_LOGW(TAG, "Json Data null");
        }
    }
}

void MqttClient::on_disconnected(esp_mqtt_event_handle_t event) {
    Client::on_disconnected(event);
    connectionStatusLiveData = MqttConnectionStatus::Disconnected;
}

void MqttClient::publishStatus() {
    SwitchStatus status = *AppSwitch::getInstance();
    ArduinoJson::DynamicJsonDocument dynamicJsonDocument(1024);
    bool statusBool = (status == SwitchStatus::Open);
    dynamicJsonDocument["data"] = statusBool;
    std::string data;
    ArduinoJson::serializeJson(dynamicJsonDocument, data);
    if (data.empty()) {
        ESP_LOGE(TAG, "publish Empty");
        return;
    }
    // At least once
    ESP_LOGE(TAG, "Publish Now Status!");
    publish(dataFilterPtr->get(), data.begin(), data.end());
}

MqttClient::~MqttClient() = default;

static std::shared_ptr<MqttClient> clientPtr;

std::shared_ptr<MqttClient> &MqttClientManager::build() {

    // TODO Add Certificate!
    idf::mqtt::BrokerConfiguration broker{
            .address = {idf::mqtt::URI{AppConfig::getInstance().getMqttHost()}, AppConfig::getInstance().getMqttPort()},
            .security =  idf::mqtt::Insecure{}
    };
    idf::mqtt::ClientCredentials credentials{};
    idf::mqtt::Configuration config{};

    // Instance
    clientPtr = std::make_shared<MqttClient>(broker, credentials, config);
    return clientPtr;
}

std::shared_ptr<MqttClient> &MqttClientManager::getClient() {
    if (!clientPtr) {
        build();
    }
    return clientPtr;
}
