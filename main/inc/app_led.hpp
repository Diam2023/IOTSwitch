#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#include "app_speech.hpp"

class AppLED : public Observer
{
private:
    const gpio_num_t pin;
    AppSpeech *sr;

public:
    AppLED(const gpio_num_t pin, AppSpeech *sr);

    void update();
};
