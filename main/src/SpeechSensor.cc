#include "SpeechSensor.h"

#include "driver/i2s_std.h"
#include "esp_err.h"
#include "esp_log.h"
#include <thread>

#include "esp_wn_iface.h"
#include "esp_wn_models.h"
#include "esp_afe_sr_models.h"
#include "esp_mn_iface.h"
#include "esp_mn_models.h"

#include "AppSwitch.h"

#define I2S_CHANNEL_NUM 1
#define I2S_CH ((i2s_port_t)1)

static const char *TAG = "Speech";

static i2s_chan_handle_t rx_handle = NULL;        // I2S rx channel handler


#define I2S_CONFIG_DEFAULT(sample_rate, channel_fmt, bits_per_chan) { \
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(sample_rate), \
        .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(bits_per_chan, channel_fmt), \
        .gpio_cfg = { \
            .mclk = GPIO_NUM_NC, \
            .bclk = GPIO_NUM_41, \
            .ws   = GPIO_NUM_42, \
            .dout = GPIO_NUM_NC, \
            .din  = GPIO_NUM_2, \
            .invert_flags = { \
                .mclk_inv = false, \
                .bclk_inv = false, \
                .ws_inv   = false, \
            }, \
        }, \
    }

static esp_err_t i2s_init(i2s_port_t i2s_num, uint32_t sample_rate, int channel_format, int bits_per_chan) {
    esp_err_t ret_val = ESP_OK;

    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(i2s_num, I2S_ROLE_MASTER);

    ret_val |= i2s_new_channel(&chan_cfg, nullptr, &rx_handle);
    i2s_std_config_t std_cfg = I2S_CONFIG_DEFAULT(sample_rate, I2S_SLOT_MODE_MONO, I2S_DATA_BIT_WIDTH_32BIT);
    std_cfg.slot_cfg.slot_mask = I2S_STD_SLOT_LEFT;
    // std_cfg.clk_cfg.mclk_multiple = EXAMPLE_MCLK_MULTIPLE;   //The default is I2S_MCLK_MULTIPLE_256. If not using 24-bit data width, 256 should be enough
    ret_val |= i2s_channel_init_std_mode(rx_handle, &std_cfg);
    ret_val |= i2s_channel_enable(rx_handle);

    return ret_val;
}

static void feed_handler(SpeechSensor *self) {
    esp_afe_sr_data_t *afe_data = self->afe_data;
    int audio_chunksize = self->afe_handle->get_feed_chunksize(afe_data);
    int nch = self->afe_handle->get_channel_num(afe_data);
    size_t samp_len = audio_chunksize;
    size_t samp_len_bytes = samp_len * I2S_CHANNEL_NUM * sizeof(int32_t);
    int32_t *i2s_buff = (int32_t *) malloc(samp_len_bytes);
    assert(i2s_buff);
    size_t bytes_read;

    while (true) {
        i2s_channel_read(rx_handle, i2s_buff, samp_len_bytes, &bytes_read, 2000 / portTICK_PERIOD_MS);

        for (int i = 0; i < samp_len; ++i) {
            i2s_buff[i] = i2s_buff[i] >> 14; // 32:8为有效位， 8:0为低8位， 全为0， AFE的输入为16位语音数据，拿29：13位是为了对语音信号放大。
        }

        self->afe_handle->feed(afe_data, (int16_t *) i2s_buff);
    }
    self->afe_handle->destroy(afe_data);
    if (i2s_buff) {
        free(i2s_buff);
        i2s_buff = nullptr;
    }
    vTaskDelete(nullptr);
}

static void detect_hander(SpeechSensor *self) {
    esp_afe_sr_data_t *afe_data = self->afe_data;
    int afe_chunksize = self->afe_handle->get_fetch_chunksize(afe_data);
    char *mn_name = esp_srmodel_filter(self->models, ESP_MN_PREFIX, ESP_MN_CHINESE);
    ESP_LOGI(TAG, "multinet:%s", mn_name);
    esp_mn_iface_t *multinet = esp_mn_handle_from_name(mn_name);
    model_iface_data_t *model_data = multinet->create(mn_name, 6000);
    esp_mn_commands_update_from_sdkconfig(multinet, model_data); // Add speech commands from sdkconfig
    int mu_chunksize = multinet->get_samp_chunksize(model_data);
    assert(mu_chunksize == afe_chunksize);

    // FILE *fp = fopen("/sdcard/out", "w");
    // if (fp == NULL) ESP_LOGE(TAG,"can not open file\n");

    //print active speech commands
    multinet->print_active_speech_commands(model_data);
    ESP_LOGI(TAG, "Ready");


    SpeechSensorStatus originStatus = (*self->speechSensorStatus);

    if (originStatus.status != SpeechStatus::NonDetect) {
        originStatus.status = SpeechStatus::NonDetect;
        self->speechSensorStatus = originStatus;
    }

    while (true) {
        afe_fetch_result_t *res = self->afe_handle->fetch(afe_data);
        if (!res || res->ret_value == ESP_FAIL) {
            ESP_LOGE(TAG, "fetch error!");
            break;
        }

//        if (res->wakeup_state == WAKENET_DETECTED) {
//            ESP_LOGI(TAG, "WAKEWORD DETECTED");
//            multinet->clean(model_data);  // clean all status of multinet
//        } else if (res->wakeup_state == WAKENET_CHANNEL_VERIFIED) {
//            ESP_LOGI(TAG, "AFE_FETCH_CHANNEL_VERIFIED, channel index: %d", res->trigger_channel_id);
//            ESP_LOGI(TAG, ">>> Say your command <<<");
//            originStatus = (*self->speechSensorStatus);
//            if (originStatus.status != SpeechStatus::Detecting) {
//                originStatus.status = SpeechStatus::Detecting;
//                self->speechSensorStatus = originStatus;
//            }
//            self->detected = true;
//            self->afe_handle->disable_wakenet(afe_data);
//        }

        originStatus = (*self->speechSensorStatus);
        originStatus.status = SpeechStatus::Detecting;
        if (originStatus.status == SpeechStatus::Detecting) {
            esp_mn_state_t mn_state = multinet->detect(model_data, res->data);

            if (mn_state == ESP_MN_STATE_DETECTING) {
                if (originStatus.command != COMMAND_NOT_DETECTED) {
                    originStatus.command = COMMAND_NOT_DETECTED;
                    self->speechSensorStatus = originStatus;
                }
                continue;
            } else if (mn_state == ESP_MN_STATE_DETECTED) {
                esp_mn_results_t *mn_result = multinet->get_results(model_data);
                for (int i = 0; i < mn_result->num; i++) {
                    ESP_LOGI(TAG, "TOP %d, command_id: %d, phrase_id: %d, string:%s prob: %f",
                             i + 1, mn_result->command_id[i], mn_result->phrase_id[i], mn_result->string,
                             mn_result->prob[i]);
                }

                if (originStatus.command != (command_word_t) mn_result->command_id[0]) {
                    originStatus.command = (command_word_t) mn_result->command_id[0];
                    if (originStatus.command == command_word_t::MENU_ON) {
                        self->switchCommandStatus = SwitchStatus::Open;
                    } else if (originStatus.command == command_word_t::MENU_OFF) {
                        self->switchCommandStatus = SwitchStatus::Close;
                    } else if (originStatus.command == command_word_t::MENU_REBOOT) {
                        esp_restart();
                    }
                    self->speechSensorStatus = originStatus;
                }
//                self->afe_handle->enable_wakenet(afe_data);

//                ESP_LOGI(TAG, ">>> Waiting to be waken up <<<");
                if (originStatus.command != COMMAND_TIMEOUT || originStatus.status != SpeechStatus::NonDetect) {
                    originStatus.status = SpeechStatus::NonDetect;
                    originStatus.command = COMMAND_TIMEOUT;
                    self->speechSensorStatus = originStatus;
                }

//                self->detected = false;
//                self->command = COMMAND_TIMEOUT;

                // self->notify();
            } else if (mn_state == ESP_MN_STATE_TIMEOUT) {
                esp_mn_results_t *mn_result = multinet->get_results(model_data);
//                ESP_LOGI(TAG, "timeout, string:%s", mn_result->string);

//                self->command = COMMAND_TIMEOUT;
//                self->afe_handle->enable_wakenet(afe_data);
//                self->detected = false;
//                ESP_LOGI(TAG, ">>> Waiting to be waken up <<<");
                if (originStatus.command != COMMAND_TIMEOUT || originStatus.status != SpeechStatus::NonDetect) {
                    originStatus.status = SpeechStatus::NonDetect;
                    originStatus.command = COMMAND_TIMEOUT;
                    self->speechSensorStatus = originStatus;
                }

                // self->notify();
            }
        }
    }
    if (model_data) {
        multinet->destroy(model_data);
        model_data = nullptr;
    }
    self->afe_handle->destroy(afe_data);
    vTaskDelete(nullptr);
}

SpeechSensor::SpeechSensor() : afe_handle(&ESP_AFE_SR_HANDLE),
                               speechSensorStatus({SpeechStatus::NonDetect, command_word_t::COMMAND_TIMEOUT}),
                               switchCommandStatus(*AppSwitch::getInstance()) {
    this->models = esp_srmodel_init("model");
    i2s_init(I2S_NUM_1, 16000, 2, 32);
    // sd_card_mount("/sdcard");
    afe_config_t afe_config = {
            .aec_init = true,
            .se_init = true,
            .vad_init = true,
            .wakenet_init = false, // Close Wakeup word detection
            .voice_communication_init = false,
            .voice_communication_agc_init = false,
            .voice_communication_agc_gain = 15,
            .vad_mode = VAD_MODE_3,
            .wakenet_model_name = nullptr,
            .wakenet_model_name_2 = nullptr,
            .wakenet_mode = DET_MODE_2CH_90,
            .afe_mode = SR_MODE_LOW_COST,
            .afe_perferred_core = 0,
            .afe_perferred_priority = 5,
            .afe_ringbuf_size = 50,
            .memory_alloc_mode = AFE_MEMORY_ALLOC_MORE_PSRAM,
            .afe_linear_gain = 1.0,
            .agc_mode = AFE_MN_PEAK_AGC_MODE_2,
            .pcm_config = {
                    .total_ch_num = 3,
                    .mic_num = 2,
                    .ref_num = 1,
                    .sample_rate = 16000,
            },
            .debug_init = true,
            .debug_hook = {{AFE_DEBUG_HOOK_MASE_TASK_IN,  nullptr},
                           {AFE_DEBUG_HOOK_FETCH_TASK_IN, nullptr}},
    };
//    afe_config.aec_init = false;
//    afe_config.se_init = false;
//    afe_config.vad_init = false;
    afe_config.afe_ringbuf_size = 10;
    afe_config.pcm_config.total_ch_num = 2;
    afe_config.pcm_config.mic_num = 1;
    afe_config.pcm_config.ref_num = 1;
    afe_config.pcm_config.sample_rate = 16000;
    afe_config.wakenet_model_name = esp_srmodel_filter(this->models, ESP_WN_PREFIX, nullptr);
    this->afe_data = this->afe_handle->create_from_config(&afe_config);
}

void SpeechSensor::run() {
    xTaskCreatePinnedToCore((TaskFunction_t) feed_handler, "AppFeed", 4 * 1024, this, 5, nullptr, 1);
    xTaskCreatePinnedToCore((TaskFunction_t) detect_hander, "AppDetect", 5 * 1024, this, 5, nullptr, 1);
}
