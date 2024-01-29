//
// Created by diam on 24-1-28.
//
#pragma once

#include "Observer.h"


#include "audio_provider.h"
#include "command_responder.h"
#include "feature_provider.h"
#include "micro_model_settings.h"
#include "model.h"
#include "recognize_commands.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_log.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/micro/system_setup.h"
#include "tensorflow/lite/schema/schema_generated.h"

namespace ms {

    constexpr int kTensorArenaSize = 30 * 1024;

    class AppMicroSpeech {
    private:
        bool runStatus = true;

        volatile bool sampleStatus = true;

        LiveData<bool> detectStatus;

        const tflite::Model *model = nullptr;
        tflite::MicroInterpreter *interpreter = nullptr;
        TfLiteTensor *model_input = nullptr;
        FeatureProvider *feature_provider = nullptr;
        RecognizeCommands *recognizer = nullptr;
        int32_t previous_time = 0;

        uint8_t tensor_arena[kTensorArenaSize];
        int8_t feature_buffer[kFeatureElementCount];
        int8_t *model_input_buffer = nullptr;

    public:
        AppMicroSpeech();

        void init();

        void loop();

        void stop();

        void cleanUp();

        LiveData<bool> &getDetectionLiveData() {
            return detectStatus;
        }

        [[nodiscard]] bool getSampleStatus() const {
            return sampleStatus;
        }

        void setSampleStatus(bool status) {
            sampleStatus = status;
        }

        static AppMicroSpeech &getInstance() {
            static AppMicroSpeech appMicroSpeech;
            return appMicroSpeech;
        }
    };

} // ms

