//#include "app_rgb.h"
//
//#include "esp_log.h"
//#include "esp_err.h"
//
//
//// 10MHz resolution, 1 tick = 0.1us (led strip needs a high resolution)
//#define LED_STRIP_RMT_RES_HZ  (10 * 1000 * 1000)
//#define LED_LIGHT_VALUE 100
//
//const static char TAG[] = "App/RGB";
//
//
//AppRGB::AppRGB(const int pin, AppSpeech *sr) : pin(pin), sr(sr), mode(led_mode_t::RGB_OFF) {
//    // LED strip ws2812 init
//    led_strip_config_t strip_config;
//    strip_config.strip_gpio_num = this->pin;   // The GPIO that connected to the LED strip's data line
//    strip_config.max_leds = 1;        // The number of LEDs in the strip
//    strip_config.led_pixel_format = LED_PIXEL_FORMAT_GRB; // Pixel format of your LED strip
//    strip_config.led_model = LED_MODEL_WS2812;            // LED strip model
//    strip_config.flags.invert_out = false;                // whether to invert the output signal
//
//    // LED strip backend configuration: RMT
//    led_strip_rmt_config_t rmt_config;
//    rmt_config.clk_src = RMT_CLK_SRC_DEFAULT;        // different clock source can lead to different power consumption
//    rmt_config.resolution_hz = LED_STRIP_RMT_RES_HZ; // RMT counter clock frequency
//    rmt_config.mem_block_symbols = 0;
//    rmt_config.flags.with_dma = false;               // DMA feature is available on ESP target like ESP32-S3
//
//    // LED Strip object handle
//    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));
//    ESP_LOGI(TAG, "Created LED strip object with RMT backend");
//}
//
//void AppRGB::setRGB(uint32_t red, uint32_t green, uint32_t blue) {
//    ESP_ERROR_CHECK(led_strip_set_pixel(this->led_strip, 0, red, green, blue));
//    ESP_ERROR_CHECK(led_strip_refresh(led_strip));
//
//    ESP_LOGI(TAG, "Set Color %ld %ld %ld", red, green, blue);
//}
//
//void AppRGB::update() {
//    if (this->sr->command > COMMAND_NOT_DETECTED) {
//
//        switch (this->sr->command) {
//            case MENU_RED:
//                mode = RGB_RED;
//                break;
//            case MENU_GREEN:
//                mode = RGB_GREEN;
//                break;
//            case MENU_BLUE:
//                mode = RGB_BLUE;
//                break;
//            case MENU_OFF:
//                mode = RGB_OFF;
//                break;
//            case MENU_ON:
//                mode = RGB_ON;
//                break;
//            default:
//                break;
//        }
//    }
//
//    // do
//    switch (mode) {
//        case RGB_ON:
//            this->setRGB(LED_LIGHT_VALUE, LED_LIGHT_VALUE, LED_LIGHT_VALUE);
//            break;
//        case RGB_OFF:
//            this->setRGB(0, 0, 0);
//            led_strip_clear(led_strip);
//            break;
//        case RGB_RED:
//            this->setRGB(LED_LIGHT_VALUE, 0, 0);
//            break;
//        case RGB_BLUE:
//            this->setRGB(0, LED_LIGHT_VALUE, 0);
//            break;
//        case RGB_GREEN:
//            this->setRGB(0, 0, LED_LIGHT_VALUE);
//            break;
//        default:
//            this->setRGB(0, 0, 0);
//            break;
//    }
//
//    vTaskDelay(10 / portTICK_PERIOD_MS);
//}