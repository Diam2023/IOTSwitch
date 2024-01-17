//#include "driver/gpio.h"

//#include "app_led.h"
//#include "app_rgb.h"
//#include "app_speech.h"
//#include "app_touch.h"
//#include "mqtt_client.h"

#include "Observer.h"
#include "TouchpadSensor.h"
#include <esp_log.h>
#include <thread>
#include "AppConfig.h"

#include <iostream>
#include <esp_system.h>
#include <nvs_flash.h>
#include <esp_netif.h>
#include <esp_pm.h>
#include <esp_spiffs.h>

#include "AppBuzzer.h"
#include "AppSwitch.h"
#include "AppRelay.h"
#include "VoltageSensor.h"


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

    AppConfig::getInstance().load();
}


extern "C" void app_main() {
//    auto *speech = new AppSpeech();

    init();

    using namespace std::chrono_literals;
    auto touch = new TouchpadSensor(AppConfig::getInstance().getTouchpadNotifyThreshold(),
                                    AppConfig::getInstance().getTouchpadTouchLimitThreshold() * 1ms);

//    auto *led = new AppLED(GPIO_NUM_14, speech);
//    auto *rgb = new AppRGB(13, speech);
//
//    speech->attach(led);
//    speech->attach(rgb);
//
//    touch->attach(led);
//    touch->attach(rgb);
//
//    speech->run();
//    // add some thing
//    touch->run();

//    Listener<int> l([](const int &i) {
//        ESP_LOGW(TAG, "changed");
//    });

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


    while (true) {


        auto voltage3 = VoltageSensor::getInstance().getVoltage(ADC1_CHANNEL_3);
        auto voltage4 = VoltageSensor::getInstance().getVoltage(ADC1_CHANNEL_4);
        auto voltage5 = VoltageSensor::getInstance().getVoltage(ADC1_CHANNEL_5);
        ESP_LOGI(TAG, "3: %lu mV \n 4: %lu mV \n 5: %lu mV", voltage3, voltage4, voltage5);
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(500ms);
    }
}
