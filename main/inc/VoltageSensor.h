//
// Created by diam on 24-1-17.
//

#ifndef IOTSWITCH_VOLTAGESENSOR_H
#define IOTSWITCH_VOLTAGESENSOR_H

#include <driver/adc.h>
#include <esp_adc_cal.h>

#include <memory>
#include <thread>

#include "Observer.h"

class VoltageSensor {
protected:
    esp_adc_cal_characteristics_t adc1_characteristics{};

    bool caliEnabled = false;

    std::shared_ptr<std::thread> readThread;

    // R3 ADC1_CH3
    LiveData<uint32_t> coreTemperatureSensorVoltage;

    // CR ADC1_CH4
    LiveData<uint32_t> outputSensorVoltage;

    // R1 ADC1_CH5
    LiveData<uint32_t> outputTemperatureSensorVoltage;
public:
    LiveData<uint32_t> &getCoreTemperatureSensorVoltage() {
        return coreTemperatureSensorVoltage;
    }

    LiveData<uint32_t> &getOutputSensorVoltage() {
        return outputSensorVoltage;
    }

    LiveData<uint32_t> &getOutputTemperatureSensorVoltage() {
        return outputTemperatureSensorVoltage;
    }

public:

    [[noreturn]] void readTask();

    uint32_t getVoltage(adc1_channel_t channel);

    VoltageSensor();

    esp_adc_cal_characteristics_t *getCharacteristicsPtr() {
        return &adc1_characteristics;
    }

    static VoltageSensor &getInstance() {
        static VoltageSensor voltageSensor;
        return voltageSensor;
    }
};


#endif //IOTSWITCH_VOLTAGESENSOR_H
