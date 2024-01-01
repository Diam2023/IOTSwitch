#include "driver/gpio.h"

#include "app_led.h"
#include "app_rgb.h"
#include "app_speech.h"
#include "app_touch.h"


extern "C" void app_main()
{
    auto *speech = new AppSpeech();

    auto *led = new AppLED(GPIO_NUM_14, speech);
    auto *rgb = new AppRGB(13, speech);

    speech->attach(led);
    speech->attach(rgb);

    speech->run();


    auto touch = new AppTouch();
    // add some
    touch->run();
}
