//
// Created by diam on 24-1-28.
//

#include "AppMicroSpeech.h"

#include <thread>

#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

namespace ms {

    [[noreturn]] static void run(void *arg) {
        AppMicroSpeech &instance = AppMicroSpeech::getInstance();
        instance.init();
        while (true) {
            instance.loop();
            using namespace std::chrono_literals;
            std::this_thread::sleep_for(50ms);
        }
    }

    AppMicroSpeech::AppMicroSpeech() {
        xTaskCreatePinnedToCore(reinterpret_cast<TaskFunction_t>(run), "AppMicroSpeech", 10 * 1024,
                                nullptr, 5, nullptr, 1);
    }

    void AppMicroSpeech::loop() {

        using namespace std::chrono_literals;

        // Fetch the spectrogram for the current time.
        const int32_t current_time = LatestAudioTimestamp();
        int how_many_new_slices = 0;
        TfLiteStatus feature_status = feature_provider->PopulateFeatureData(
                previous_time, current_time, &how_many_new_slices);
        if (feature_status != kTfLiteOk) {
            MicroPrintf("Feature generation failed");
            return;
        }
        previous_time = current_time;
        // If no new audio samples have been received since last time, don't bother
        // running the network model.
        if (how_many_new_slices == 0) {
            return;
        }

        // Copy feature buffer to input tensor
        for (int i = 0; i < kFeatureElementCount; i++) {
            model_input_buffer[i] = feature_buffer[i];
        }

        // Run the model on the spectrogram input and make sure it succeeds.
        TfLiteStatus invoke_status = interpreter->Invoke();
        if (invoke_status != kTfLiteOk) {
            MicroPrintf("Invoke failed");
            return;
        }

        // Obtain a pointer to the output tensor
        TfLiteTensor *output = interpreter->output(0);
        // Determine whether a command was recognized based on the output of inference
        const char *found_command = nullptr;
        uint8_t score = 0;
        bool is_new_command = false;
        TfLiteStatus process_status = recognizer->ProcessLatestResults(
                output, current_time, &found_command, &score, &is_new_command);
        if (process_status != kTfLiteOk) {
            MicroPrintf("RecognizeCommands::ProcessLatestResults() failed");
            return;
        }
        // Do something based on the recognized command. The default implementation
        // just prints to the error console, but you should replace this with your
        // own function for a real application.
        RespondToCommand(current_time, found_command, score, is_new_command);
        if (strcmp(found_command, kCategoryLabels[2]) == 0) {
            detectStatus = true;
        }
    }

    void AppMicroSpeech::stop() {
    }

    void AppMicroSpeech::cleanUp() {
    }

    void AppMicroSpeech::init() {

        model = tflite::GetModel(g_model);
        if (model->version() != TFLITE_SCHEMA_VERSION) {
            MicroPrintf("Model provided is schema version %d not equal to supported "
                        "version %d.", model->version(), TFLITE_SCHEMA_VERSION);
            return;
        }

        static tflite::MicroMutableOpResolver<22> micro_op_resolver;
        if (micro_op_resolver.AddDepthwiseConv2D() != kTfLiteOk) {
            return;
        }
        if (micro_op_resolver.AddFullyConnected() != kTfLiteOk) {
            return;
        }
        if (micro_op_resolver.AddSoftmax() != kTfLiteOk) {
            return;
        }
        if (micro_op_resolver.AddReshape() != kTfLiteOk) {
            return;
        }
        if (micro_op_resolver.AddQuantize() != kTfLiteOk) {
            return;
        }
        if (micro_op_resolver.AddConv2D() != kTfLiteOk) {
            return;
        }

        // static tflite::AllOpsResolver resolver;

        // Build an interpreter to run the model with.
        static tflite::MicroInterpreter static_interpreter(
                model, micro_op_resolver, tensor_arena, kTensorArenaSize);
        interpreter = &static_interpreter;

        // Allocate memory from the tensor_arena for the model's tensors.
        TfLiteStatus allocate_status = interpreter->AllocateTensors();
        if (allocate_status != kTfLiteOk) {
            MicroPrintf("AllocateTensors() failed");
            return;
        }

        // Get information about the memory area to use for the model's input.
        model_input = interpreter->input(0);
        if ((model_input->dims->size != 2) || (model_input->dims->data[0] != 1) ||
            (model_input->dims->data[1] !=
             (kFeatureSliceCount * kFeatureSliceSize)) ||
            (model_input->type != kTfLiteInt8)) {
            MicroPrintf("Bad input tensor parameters in model");
            return;
        }
        model_input_buffer = model_input->data.int8;

        // Prepare to access the audio spectrograms from a microphone or other source
        // that will provide the inputs to the neural network.
        // NOLINTNEXTLINE(runtime-global-variables)
        static FeatureProvider static_feature_provider(kFeatureElementCount,
                                                       feature_buffer);
        feature_provider = &static_feature_provider;

        static RecognizeCommands static_recognizer;
        recognizer = &static_recognizer;

        previous_time = 0;
    }
} // ms