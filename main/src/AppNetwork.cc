//
// Created by diam on 24-1-18.
//

#include "AppNetwork.h"

#include "Tools.h"
#include "OkWifi.h"
#include "ProvServerScanner.h"

#include "AppConfig.h"

#include <lwip/err.h>
#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_log.h>
#include <nvs_flash.h>
#include <esp_event_cxx.hpp>

static const char *TAG = "AppNetWork";

AppNetwork::AppNetwork() : ssid(AppConfig::getInstance().getWifiSsid()), pwd(AppConfig::getInstance().getWifiPwd()) {
}

static bool start() {
    if (!AppNetwork::getInstance().getSsid().empty()) {
        return AppNetwork::getInstance().connect();
    }

    // Custom Prov Service Name
    ok_wifi::OkWifi::getInstance().setEnableBatchProv(false);
    ok_wifi::WifiProv::getInstance().setServiceName("IOTSwitch");

    ok_wifi::OkWifi::getInstance().init();

    ok_wifi::OkWifi::getInstance().waitExit();

    auto provResult = ok_wifi::WifiProv::getInstance().getProvResult();

    if ((ok_wifi::OkWifi::getInstance().getStatus() == ok_wifi::OkWifiStartMode::ModeCompleted) &&
        (provResult.getResult() == ok_wifi::ProvResultStatus::ResOk)) {
        ESP_LOGI(TAG, "Tack SSID: %s PWD %s", provResult.getSsid().c_str(),
                 provResult.getPwd().c_str());
        AppNetwork::getInstance().setSsid(provResult.getSsid());
        AppNetwork::getInstance().setPwd(provResult.getPwd());
        return AppNetwork::getInstance().connect();
    } else {
        ESP_LOGE(TAG, "Prov Error!!!");
        return false;
    }
}

static void event_handler(void *arg, esp_event_base_t event_base,
                          int32_t event_id, void *event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        AppNetwork::getInstance().getNetworkStatusLiveData() = NetworkStatus::Disconnected;
        esp_wifi_connect();
        ESP_LOGW(TAG, "Retry Connect Wifi");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        auto *event = (ip_event_got_ip_t *) event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        AppNetwork::getInstance().getNetworkStatusLiveData() = NetworkStatus::Connected;

        if ((AppConfig::getInstance().getWifiSsid() != AppNetwork::getInstance().getSsid()) ||
            (AppConfig::getInstance().getWifiPwd() != AppNetwork::getInstance().getPwd())) {
            // Setting SSID And PWD
            ESP_LOGI(TAG, "Setting SSID And PWD");
            AppConfig::getInstance().setWifiSsid(AppNetwork::getInstance().getSsid());
            AppConfig::getInstance().setWifiPwd(AppNetwork::getInstance().getPwd());
            AppConfig::getInstance().write();
        }
    }
}

static esp_event_handler_instance_t instance_any_id;
static esp_event_handler_instance_t instance_got_ip;

bool AppNetwork::connect() {
    using namespace std::chrono_literals;

    ok_wifi::ProvServerScanner::getInstance().setServerSsid(ssid);
    ok_wifi::ProvServerScanner::getInstance().init();

    try {
        ok_wifi::ProvServerScanner::getInstance().wait(5);
    } catch (const idf::event::EventException &e) {
        ESP_LOGE(TAG, "Connection Not Found !!!");
        ssid = "";
        *AppNetwork::getInstance().getNetworkStatusLiveData() = NetworkStatus::NotFound;
        return false;
    }

    ok_wifi::ProvServerScanner::getInstance().deinit();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    esp_netif_create_default_wifi_sta();

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
                                                        nullptr,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &event_handler,
                                                        nullptr,
                                                        &instance_got_ip));

    wifi_config_t wifi_config;

    bzero(&wifi_config, sizeof(wifi_config_t));
    ok_wifi::stringToUint(wifi_config.sta.ssid, ssid);
    ok_wifi::stringToUint(wifi_config.sta.password, pwd);
    ESP_LOGI(TAG, "Prepare To Connect ProvServer SSID: %s PWD: %s", wifi_config.sta.ssid, wifi_config.sta.password);
    wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA_WPA2_PSK;
    // wifi_config.sta.pmf_cfg.capable = false;
    // wifi_config.sta.pmf_cfg.required = false;
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
//    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_FLASH));
    ESP_ERROR_CHECK(esp_wifi_disable_pmf_config(WIFI_IF_STA));

    ESP_ERROR_CHECK(esp_wifi_start());
    auto res = esp_wifi_connect();
    if (res != ESP_OK) {
        char err_msg[1024];
        esp_err_to_name_r(res, err_msg, 1024);
        ESP_LOGE(TAG, "err res: %s", err_msg);
        ESP_LOGI(TAG, "wifi_init_sta finished.");
        return false;
    }

    return true;
}


static QueueDefinition *binarySemaphore = nullptr;

static void startSyncWhileSuccessful() {
    bool startResult = false;
    while (!startResult) {
        startResult = start();
        if (*AppNetwork::getInstance().getNetworkStatusLiveData() == NetworkStatus::NotFound) {
            static int retryCounter = 1;
            retryCounter++;
            if (retryCounter > 3) {
                ESP_LOGE(TAG, "Error Prov Device");
                esp_restart();
            }
            continue;
        } else {
            ESP_LOGE(TAG, "Wifi Init Error!");
        }
    }
    if (binarySemaphore != nullptr) {
        xSemaphoreGive(binarySemaphore);
    }
    vTaskDelete(nullptr);
}

static TaskHandle_t handler;

void AppNetwork::asyncStart() {
    xTaskCreatePinnedToCore((TaskFunction_t) startSyncWhileSuccessful, "AppNetwork", 5 * 1024, nullptr, 3, &handler, 0);
    ESP_LOGI(TAG, "AppNetwork Created");
}

void AppNetwork::syncStart() {
    if (binarySemaphore != nullptr) {
        ESP_LOGE(TAG, "Error Launch Another TAsk Running!");
        return;
    }
    asyncStart();
    binarySemaphore = xSemaphoreCreateBinary();
    xSemaphoreTake(binarySemaphore, portMAX_DELAY);
    binarySemaphore = nullptr;
    ESP_LOGI(TAG, "AppNetwork Created");
}
