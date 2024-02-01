//
// Created by diam on 24-1-17.
//

#include "VoltageSensor.h"

#include <esp_log.h>

static const char *TAG = "VoltageSensor";

// TODO Change to DMA Mode


VoltageSensor::VoltageSensor() {
    auto ret = esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_TP_FIT);
    if (ret == ESP_ERR_NOT_SUPPORTED) {
        ESP_LOGW(TAG, "Calibration scheme not supported, skip software calibration");
    } else if (ret == ESP_ERR_INVALID_VERSION) {
        ESP_LOGW(TAG, "eFuse not burnt, skip software calibration");
    } else if (ret == ESP_OK) {
        caliEnabled = true;
        esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_12, ADC_WIDTH_BIT_12, 0, &adc1_characteristics);
    } else {
        ESP_LOGE(TAG, "Invalid arg");
    }

    ESP_ERROR_CHECK(adc1_config_width(ADC_WIDTH_BIT_12));

    ESP_ERROR_CHECK(adc1_config_channel_atten(ADC1_CHANNEL_3, ADC_ATTEN_DB_12));
    ESP_ERROR_CHECK(adc1_config_channel_atten(ADC1_CHANNEL_4, ADC_ATTEN_DB_12));
    ESP_ERROR_CHECK(adc1_config_channel_atten(ADC1_CHANNEL_5, ADC_ATTEN_DB_12));

    readThread = std::make_shared<std::thread>(&VoltageSensor::readTask, this);
}

uint32_t VoltageSensor::getVoltage(adc1_channel_t channel) {
    uint32_t resultVoltage = adc1_get_raw(channel);
    if (caliEnabled) {
        resultVoltage = esp_adc_cal_raw_to_voltage(resultVoltage, &adc1_characteristics);
    }
    return resultVoltage;
}

[[noreturn]] void VoltageSensor::readTask() {
    using namespace std::chrono_literals;

    ESP_LOGI(TAG, "Read Task Start");

    while (true) {
        uint32_t coreTempVol = getVoltage(ADC1_CHANNEL_3);
        uint32_t outputVol = getVoltage(ADC1_CHANNEL_4);
        uint32_t outputTempVol = getVoltage(ADC1_CHANNEL_5);

//        ESP_LOGE(TAG, "o: %lu mV, ct: %lu mV, ot: %lu mV;", outputVol, coreTempVol, outputTempVol);

        if (*coreTemperatureSensorVoltage != coreTempVol) {
            coreTemperatureSensorVoltage = coreTempVol;
        }

        if (*outputSensorVoltage != outputVol) {
            ESP_LOGW(TAG, "Update outputSensor Voltage");
            outputSensorVoltage = outputVol;
        }

        if (*outputTemperatureSensorVoltage != outputTempVol) {
            outputTemperatureSensorVoltage = outputTempVol;
        }

        std::this_thread::sleep_for(700ms);
    }

    ESP_LOGW(TAG, "Read Task End");
}