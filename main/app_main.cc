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

static const char *TAG = "MAIN";

class NumListener : public Listener<int> {
public:
    NumListener() = default;

    void onListener(const int &i) override {
        ESP_LOGW(TAG, "num changed %d", i);
    }
};

extern "C" void app_main() {
//    auto *speech = new AppSpeech();
    auto touch = new TouchpadSensor();
//
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

    touch->getTouchStatusLiveData().append([](auto &status) {
        ESP_LOGW(TAG, "Status Changed!");
    });

    touch->run();
    int i = 1;
    while (true) {
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(2s);
    }
}
