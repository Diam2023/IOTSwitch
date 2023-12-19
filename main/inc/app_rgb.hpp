#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#include "led_strip.h"

#include "app_speech.hpp"

enum led_mode_t
{
    RGB_OFF = 0,
    RGB_ON = 1,
    RGB_RED = 2,
    RGB_BLUE = 3,
    RGB_GREEN = 4,
};


class AppRGB : public Observer
{
private:
    const int pin;
    led_strip_handle_t led_strip;
    AppSpeech *sr;
    led_mode_t mode;


public:
    AppRGB(const int pin, AppSpeech *sr);

    void update();

    /**
     * @brief Range 0(0%) ~ 255(0%)
     * 
     * @param red red
     * @param green green
     * @param blue blue
     */
    void setRGB(uint32_t red, uint32_t green, uint32_t blue);
};
