#include "TouchpadSensor.h"
#include <esp_log.h>
#include <thread>
#include "AppConfig.h"

#include <iostream>
#include <esp_system.h>
#include <nvs_flash.h>
#include <esp_netif.h>
#include <esp_spiffs.h>

#include "AppBuzzer.h"
#include "AppSwitch.h"
#include "AppRelay.h"
#include "VoltageSensor.h"
#include "SwitchStatusSensor.h"
#include "TemperatureSensor.h"
#include "SpeechSensor.h"
#include "AppRgb.h"

#include "AppNetwork.h"
#include "MqttClient.h"

using namespace std::chrono_literals;

static const char *TAG = "MAIN";

void init() {
    ESP_LOGI(TAG, "Initializing SPIFFS");
    esp_vfs_spiffs_conf_t conf = {.base_path = "/spiffs",
            .partition_label = "config",
            .max_files = 5,
            .format_if_mount_failed = true};

    // Use settings defined above to initialize and mount SPIFFS filesystem.
    // Note: esp_vfs_spiffs_register is an all-in-one convenience function.
    esp_err_t ret = esp_vfs_spiffs_register(&conf);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount or format filesystem");
        } else if (ret == ESP_ERR_NOT_FOUND) {
            ESP_LOGE(TAG, "Failed to find SPIFFS partition");
        } else {
            ESP_LOGE(TAG,
                     "Failed to initialize SPIFFS (%s)",
                     esp_err_to_name(ret));
        }

        esp_restart();
    }

    size_t total = 0, used = 0;
    ret = esp_spiffs_info("config", &total, &used);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG,
                 "Failed to get SPIFFS partition information (%s)",
                 esp_err_to_name(ret));
    } else {
        ESP_LOGI(TAG, "Partition size: total: %d, used: %d", total, used);
    }

    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());

    setenv("TZ", "CST-8", 1);
    tzset();

    esp_event_loop_create_default();

    AppConfig::getInstance().load();
}


extern "C" void app_main() {
    init();

    AppNetwork::getInstance().getNetworkStatusLiveData().append([](auto s) {
        if (s == NetworkStatus::Connected) {
            ESP_LOGI(TAG, "Connected!");
            MqttClientManager::getClient()->getConnectionStatusLiveData().append([](auto s) {
                if (s == MqttConnectionStatus::MqttConnected) {
                    // Online Flag
                    MqttClientManager::getClient()->publishStatus();
                }
            });
        } else {
            ESP_LOGI(TAG, "Err Connection!");
        }
    });


    AppNetwork::asyncStart();

    auto touch = new TouchpadSensor(AppConfig::getInstance().getTouchpadNotifyThreshold(),
                                    AppConfig::getInstance().getTouchpadTouchLimitThreshold() * 1ms);

    AppSwitch::getInstance().append(AppBuzzer::getInstance());
    AppSwitch::getInstance().append(AppRelay::getInstance());

    touch->getTouchStatusLiveData().append([](auto &status) {
        if (status == TouchStatus::TOUCHING) {
            ESP_LOGW(TAG, "Touching");
            AppSwitch::getInstance() =
                    (*AppSwitch::getInstance() == SwitchStatus::Open)
                    ? SwitchStatus::Close
                    : SwitchStatus::Open;
        }
    });

    touch->run();

    ESP_LOGI(TAG, "Device SN: %s", AppConfig::getInstance().getDeviceSerialNumber().c_str());
    ESP_LOGI(TAG, "Version: %s", AppConfig::getInstance().getConfigVersion().c_str());

    AppConfig::getInstance().write();

    VoltageSensor::getInstance().getOutputTemperatureSensorVoltage().append(TemperatureSensor::getInstance());
    VoltageSensor::getInstance().getCoreTemperatureSensorVoltage().append(TemperatureSensor::getInstance());
    VoltageSensor::getInstance().getOutputSensorVoltage().append(SwitchStatusSensor::getInstance());

    // Important
    SpeechSensor::getInstance().run();
    SpeechSensor::getInstance().switchCommandStatus.append([](auto s) {
        if (s == SwitchStatus::Open) {
            AppSwitch::getInstance() = SwitchStatus::Open;
//            if (*AppSwitch::getInstance() == SwitchStatus::Close) {
//            }
        } else {
//            if (*AppSwitch::getInstance() == SwitchStatus::Open) {
                AppSwitch::getInstance() = SwitchStatus::Close;
//            }
        }
    });

    // Launch Beep
    AppBuzzer::getInstance().beepMicroTimeAsync({1, 50});

    // REMOVE Detect beep
//    SpeechSensor::getInstance().speechSensorStatus.append(AppRgb::getInstance());
//    SpeechSensor::getInstance().speechSensorStatus.append([](const SpeechSensorStatus &s) {
//        if ((s.status == SpeechStatus::Detecting) && (s.command < 0)) {
//            AppBuzzer::getInstance().beepMicroTimeAsync({1, 50});
//        }
//    });

//    AppRgb::getInstance().setRGB(100, 100, 100);

    while (true) {
        std::this_thread::sleep_for(500ms);
    }
}
