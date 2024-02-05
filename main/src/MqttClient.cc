//
// Created by diam on 24-1-17.
//

#include "MqttClient.h"
#include "AppSwitch.h"

#include <sstream>
#include <string>
#include <utility>
#include <esp_log.h>
#include <thread>
#include <fstream>
#include <mutex>

#include "VoltageSensor.h"

static const char *TAG = "MqttClient";

MqttClient::MqttClient(const idf::mqtt::BrokerConfiguration &broker, const idf::mqtt::ClientCredentials &credentials,
                       const idf::mqtt::Configuration &config) : idf::mqtt::Client(broker, credentials, config),
                                                                 connectionStatusLiveData(
                                                                         MqttConnectionStatus::MqttInit) {
    std::stringstream actionFilterString;
    std::stringstream configFilterString;
    std::stringstream statusFilterString;
    std::stringstream settingFilterString;
    std::stringstream warningFilterString;

    // Sub
    actionFilterString << "devices/" << AppConfig::getInstance().getDeviceSerialNumber() << "/action";
    settingFilterString << "devices/" << AppConfig::getInstance().getDeviceSerialNumber() << "/setting";

    // Pub
    statusFilterString << "devices/" << AppConfig::getInstance().getDeviceSerialNumber() << "/status";
    configFilterString << "devices/" << AppConfig::getInstance().getDeviceSerialNumber() << "/config";
    warningFilterString << "devices/" << AppConfig::getInstance().getDeviceSerialNumber() << "/warning";

    actionFilterPtr = std::make_unique<idf::mqtt::Filter>(actionFilterString.str());
    settingFilterPtr = std::make_unique<idf::mqtt::Filter>(settingFilterString.str());

    statusFilterPtr = std::make_unique<idf::mqtt::Filter>(statusFilterString.str());
    configFilterPtr = std::make_unique<idf::mqtt::Filter>(configFilterString.str());
    warningFilterPtr = std::make_unique<idf::mqtt::Filter>(warningFilterString.str());

    ESP_LOGI(TAG, "Subscribe Data Filter: %s", actionFilterPtr->get().c_str());
    ESP_LOGI(TAG, "Subscribe Config Filter: %s", configFilterPtr->get().c_str());
};

void MqttClient::on_connected(esp_mqtt_event_handle_t event) {
    subscribe(actionFilterPtr->get(), idf::mqtt::QoS::AtMostOnce);
    subscribe(configFilterPtr->get(), idf::mqtt::QoS::AtMostOnce);
    connectionStatusLiveData = MqttConnectionStatus::MqttConnected;
}

void MqttClient::on_data(esp_mqtt_event_handle_t event) {
    std::string jsonConfig(event->data, event->data_len);
    if (jsonConfig.empty()) {
        ESP_LOGW(TAG, "Ignore MqttClient Config Empty Data");
        return;
    }
    if (settingFilterPtr->match(event->topic, event->topic_len)) {
        AppConfig::getInstance().loadJsonConfig(jsonConfig);
        AppConfig::getInstance().write();
    } else if (actionFilterPtr->match(event->topic, event->topic_len)) {
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
            ESP_LOGI(TAG, "Get Config");
            publishConfig();
        }
    }
}

void MqttClient::on_disconnected(esp_mqtt_event_handle_t event) {
    Client::on_disconnected(event);
    connectionStatusLiveData = MqttConnectionStatus::MqttDisconnected;
}

ArduinoJson::StaticJsonDocument<1024> staticJsonDocument;

void MqttClient::publishStatus() {
    if (*connectionStatusLiveData != MqttConnectionStatus::MqttConnected) {
        ESP_LOGW(TAG, "Mqtt Not Connected");
        return;
    }

    SwitchStatus status = *AppSwitch::getInstance();
    bool statusBool = (status == SwitchStatus::Open);
    staticJsonDocument.clear();
    staticJsonDocument["status"] = statusBool;

    static uint32_t coreTemp;
    static uint32_t switchTemp;
    coreTemp = *VoltageSensor::getInstance().getCoreTemperatureSensorVoltage();
    switchTemp = *VoltageSensor::getInstance().getOutputTemperatureSensorVoltage();
    staticJsonDocument["coreTemperature"] = coreTemp;
    staticJsonDocument["switchTemperature"] = switchTemp;

    std::string data;
    ArduinoJson::serializeJson(staticJsonDocument, data);
    if (data.empty()) {
        ESP_LOGE(TAG, "publish Empty");
        return;
    }
    // At least once
    ESP_LOGI(TAG, "Publish Now Status!");
    publish(statusFilterPtr->get(), data.begin(), data.end());
}

void MqttClient::publishConfig() {
    if (*connectionStatusLiveData != MqttConnectionStatus::MqttConnected) {
        ESP_LOGW(TAG, "Mqtt Not Connected");
        return;
    }
    AppConfig::getInstance().write();

    std::ifstream ifs(g_pConfigFilePath, std::ifstream::in);
    std::stringstream buffer;
    buffer << ifs.rdbuf();
    std::string jsonString(buffer.str());

    publish(configFilterPtr->get(), jsonString.begin(), jsonString.end());
}

void MqttClient::publishWarning(const std::string &msg) {
    if (*connectionStatusLiveData != MqttConnectionStatus::MqttConnected) {
        ESP_LOGW(TAG, "Mqtt Not Connected");
        return;
    }
    staticJsonDocument.clear();
    staticJsonDocument["message"] = msg;
    std::string jsonString;
    ArduinoJson::serializeJson(staticJsonDocument, jsonString);
    // At least once
    ESP_LOGI(TAG, "Publish Warning!");
    publish(warningFilterPtr->get(), jsonString.begin(), jsonString.end());

}

MqttClient::~MqttClient() = default;

static std::shared_ptr<MqttClient> clientPtr;

static QueueDefinition *binarySemaphore;

void MqttClientManager::build() {

    // TODO Add Certificate!
    idf::mqtt::BrokerConfiguration broker{
            .address = {idf::mqtt::URI{AppConfig::getInstance().getMqttHost()}, AppConfig::getInstance().getMqttPort()},
            .security =  idf::mqtt::Insecure{}
    };
    idf::mqtt::ClientCredentials credentials{};
    idf::mqtt::Configuration config{};

    // Instance
    clientPtr = std::make_shared<MqttClient>(broker, credentials, config);
    xSemaphoreGive(binarySemaphore);
    vTaskDelete(nullptr);
}


static std::mutex entryLock;

std::shared_ptr<MqttClient> &MqttClientManager::getClient() {
    entryLock.lock();
    if (!clientPtr) {
        TaskHandle_t handler;
        xTaskCreatePinnedToCore((TaskFunction_t) build, "MqttBuilder", 5 * 1024, nullptr, 5, &handler, 0);
        binarySemaphore = xSemaphoreCreateBinary();
        xSemaphoreTake(binarySemaphore, portMAX_DELAY);
        ESP_LOGI(TAG, "MqttClient Created");
    }
    entryLock.unlock();
    return clientPtr;
}
