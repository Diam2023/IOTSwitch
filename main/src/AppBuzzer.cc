#include "AppBuzzer.h"
#include "AppConfig.h"

#include <esp_log.h>

#include <driver/ledc.h>
#include <thread>

const static char *TAG = "AppBuzzer";

#define LEDC_TIMER              LEDC_TIMER_0
#define LEDC_MODE               LEDC_LOW_SPEED_MODE
#define LEDC_CHANNEL            LEDC_CHANNEL_0
#define LEDC_DUTY_RES           LEDC_TIMER_13_BIT // Set duty resolution to 13 bits
#define LEDC_DUTY               (4096) // Set duty to 50%. (2 ** 13) * 50% = 4096
#define LEDC_FREQUENCY          (4000) // Frequency in Hertz. Set frequency at 4 kHz

AppBuzzer::AppBuzzer() : targetPin(13), beepThread(), beepTasks() {
    // Prepare and then apply the LEDC PWM timer configuration
    ledc_timer_config_t ledc_timer = {
            .speed_mode       = LEDC_MODE,
            .duty_resolution  = LEDC_DUTY_RES,
            .timer_num        = LEDC_TIMER,
            .freq_hz          = LEDC_FREQUENCY,  // Set output frequency at 4 kHz
            .clk_cfg          = LEDC_AUTO_CLK
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    // Prepare and then apply the LEDC PWM channel configuration
    ledc_channel_config_t ledc_channel = {
            .gpio_num       = targetPin,
            .speed_mode     = LEDC_MODE,
            .channel        = LEDC_CHANNEL,
            .intr_type      = LEDC_INTR_DISABLE,
            .timer_sel      = LEDC_TIMER,
            .duty           = 0, // Set duty to 0%
            .hpoint         = 0,
            .flags          = {0}
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));

    beepTasks = std::make_shared<std::deque<BeepTask>>();
    beepThread = std::make_shared<std::thread>(&AppBuzzer::beepTask, this);
}

void AppBuzzer::beepMicroTimeSync(const BeepTask &t) {
    using namespace std::chrono_literals;
    for (int i = 0; i < t.count; i++) {
        ESP_LOGD(TAG, "Beep !");

        ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, LEDC_DUTY);
        ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
        std::this_thread::sleep_for(t.timeDelay * 1ms);

        ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, 0);
        ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
        std::this_thread::sleep_for(t.timeDelay * 1ms);
    }

}


void AppBuzzer::onListener(const SwitchStatus &d) {
    if (d == SwitchStatus::Open) {
        beepMicroTimeAsync({2, 100});
    } else {
        beepMicroTimeAsync({1, 100});
    }
}

[[noreturn]] void AppBuzzer::beepTask() {
    ESP_LOGI(TAG, "Beep Task Start");
    using namespace std::chrono_literals;
    while (true) {
        if (!beepTasks->empty()) {
            auto t = beepTasks->front();

            if (!AppConfig::getInstance().getBeepMute()) {
                beepMicroTimeSync(t);
            } else {
                ESP_LOGW(TAG, "Mute");
            }

            beepTasks->pop_front();

        } else {
            std::this_thread::sleep_for(100ms);
        }
    }
    ESP_LOGW(TAG, "Beep Task End");
}

void AppBuzzer::beepMicroTimeAsync(BeepTask &&t) {
    beepTasks->push_back(t);
}
