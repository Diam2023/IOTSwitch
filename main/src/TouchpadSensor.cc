//
// Created by diam on 24-1-1.
//
#include "TouchpadSensor.h"

#include <thread>

#include <esp_log.h>

#include <driver/touch_pad.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

static const char *TAG = "App/Touch";


TouchpadSensor::TouchpadSensor(int64_t notifyThreshold_, std::chrono::microseconds touchLimitThreshold_)
        : notifyThreshold(
        notifyThreshold_), touchLimitThreshold(touchLimitThreshold_) {

    touch_pad_init();
    touch_pad_config(TOUCH_PAD_NUM7);
    touch_pad_set_measurement_interval(TOUCH_PAD_SLEEP_CYCLE_DEFAULT);
    touch_pad_set_charge_discharge_times(TOUCH_PAD_MEASURE_CYCLE_DEFAULT);
    touch_pad_set_voltage(TOUCH_PAD_HIGH_VOLTAGE_THRESHOLD, TOUCH_PAD_LOW_VOLTAGE_THRESHOLD,
                          TOUCH_PAD_ATTEN_VOLTAGE_THRESHOLD);
    touch_pad_set_idle_channel_connect(TOUCH_PAD_IDLE_CH_CONNECT_DEFAULT);

    touch_pad_set_cnt_mode(TOUCH_PAD_NUM7, TOUCH_PAD_SLOPE_DEFAULT, TOUCH_PAD_TIE_OPT_DEFAULT);

    touch_pad_denoise_t denoise = {
            .grade = TOUCH_PAD_DENOISE_BIT4,
            .cap_level = TOUCH_PAD_DENOISE_CAP_L4,
    };

    touch_pad_denoise_set_config(&denoise);
    touch_pad_denoise_enable();

    ESP_LOGI(TAG, "Denoise function init");

    touch_pad_set_fsm_mode(TOUCH_FSM_MODE_TIMER);
    touch_pad_fsm_start();
    // Init Touch Pad
}

[[noreturn]] static void touch_handler(TouchpadSensor *self) {

    uint32_t touch_value;

    uint32_t pre_touch_value;
    touch_pad_read_raw_data(TOUCH_PAD_NUM7, &touch_value);
    pre_touch_value = touch_value;

    std::this_thread::sleep_for(1s);

    ESP_LOGI(TAG, "Touch Start");

    while (true) {
        touch_pad_read_raw_data(TOUCH_PAD_NUM7, &touch_value);

        int64_t change = (int64_t) touch_value - pre_touch_value;
        pre_touch_value = touch_value;
        if (change >= self->getNotifyThreshold()) {
            if (self->getTouchStatus() != TouchStatus::TOUCHING) {
                self->setTouchStatus(TouchStatus::TOUCHING);
                ESP_LOGD(TAG, "Change to Touching");
            }
            ESP_LOGD(TAG, "Touch Change %lud", touch_value);
        } else {
            if (self->getTouchStatus() != TouchStatus::NORMAL) {
                self->setTouchStatus(TouchStatus::NORMAL);
                ESP_LOGD(TAG, "Change to Normal");
            }
        }

        ESP_LOGD(TAG, "Touch Val %lud", touch_value);
        std::this_thread::sleep_for(self->getTouchLimitThreshold());
    }
}

void TouchpadSensor::run() {
    xTaskCreatePinnedToCore((TaskFunction_t) touch_handler, "AppTouch", 4 * 1024, this, 5, nullptr, 0);
}

