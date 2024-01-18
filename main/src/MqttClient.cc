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
                                                                 connectionStatusLiveData(
                                                                         MqttConnectionStatus::MqttInit) {
    std::stringstream dataFilterString;
    std::stringstream configFilterString;
    std::stringstream statusFilterString;

    dataFilterString << "devices/" << AppConfig::getInstance().getDeviceSerialNumber() << "/data";
    configFilterString << "devices/" << AppConfig::getInstance().getDeviceSerialNumber() << "/config";
    statusFilterString << "devices/" << AppConfig::getInstance().getDeviceSerialNumber() << "/status";

    dataFilterPtr = std::make_unique<idf::mqtt::Filter>(dataFilterString.str());
    configFilterPtr = std::make_unique<idf::mqtt::Filter>(configFilterString.str());
    statusFilterPtr = std::make_unique<idf::mqtt::Filter>(statusFilterString.str());

    ESP_LOGI(TAG, "Subscribe Data Filter: %s", dataFilterPtr->get().c_str());
    ESP_LOGI(TAG, "Subscribe Config Filter: %s", configFilterPtr->get().c_str());
};

void MqttClient::on_connected(esp_mqtt_event_handle_t event) {
    subscribe(dataFilterPtr->get(), idf::mqtt::QoS::AtMostOnce);
    subscribe(configFilterPtr->get(), idf::mqtt::QoS::AtMostOnce);
    connectionStatusLiveData = MqttConnectionStatus::MqttConnected;
}

void MqttClient::on_data(esp_mqtt_event_handle_t event) {
    std::string jsonConfig(event->data, event->data_len);
    if (jsonConfig.empty()) {
        ESP_LOGW(TAG, "Ignore MqttClient Config Empty Data");
        return;
    }
    if (configFilterPtr->match(event->topic, event->topic_len)) {
        AppConfig::getInstance().loadJsonConfig(jsonConfig);
        AppConfig::getInstance().write();
    } else if (dataFilterPtr->match(event->topic, event->topic_len)) {
        ArduinoJson::DynamicJsonDocument dynamicJsonDocument(1024);
        auto error = ArduinoJson::deserializeJson(dynamicJsonDocument, jsonConfig);
        if (error) {
            ESP_LOGW(TAG, "Deserialize Json Data Error");
            return;
        }

        std::string action = dynamicJsonDocument["action"];
        if (action.empty()) {
            return;
        }
        if (action == "setStatus") {
            auto switchJson = dynamicJsonDocument["status"];
            if (!switchJson.isNull()) {
                auto setStatus = ((bool) switchJson) ? SwitchStatus::Open : SwitchStatus::Close;
                // Skip Callback loop!
                AppSwitch::getInstance().setWithoutPublish(setStatus);
            } else {
                ESP_LOGW(TAG, "Json Data null");
            }
        } else if (action == "restart") {
            ESP_LOGW(TAG, "Restart");
            AppConfig::getInstance().write();
            esp_restart();
        } else if (action == "getStatus") {
            ESP_LOGI(TAG, "Get Status");
            publishStatus();
        } else if (action == "getConfig") {
            // TODO Wait Complete
        }
    }
}

void MqttClient::on_disconnected(esp_mqtt_event_handle_t event) {
    Client::on_disconnected(event);
    connectionStatusLiveData = MqttConnectionStatus::MqttDisconnected;
}

void MqttClient::publishStatus() {
    if (*connectionStatusLiveData != MqttConnectionStatus::MqttConnected) {
        ESP_LOGW(TAG, "Mqtt Not Connected");
        return;
    }

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
    ESP_LOGI(TAG, "Publish Now Status!");
    publish(statusFilterPtr->get(), data.begin(), data.end());
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
