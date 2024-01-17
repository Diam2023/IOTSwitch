//
// Created by diam on 24-1-17.
//

#ifndef IOTSWITCH_COMMON_H
#define IOTSWITCH_COMMON_H


enum SwitchStatus {
    Open,
    Close
};

enum TouchStatus {
    NORMAL,
    TOUCHING,
};

typedef enum {
    COMMAND_TIMEOUT = -2,
    COMMAND_NOT_DETECTED = -1,

    MENU_REBOOT = 0,
    MENU_RED = 1,
    MENU_BLUE = 2,
    MENU_GREEN = 3,
    MENU_ON = 4,
    MENU_OFF = 5,
} command_word_t;

enum SpeechStatus {
    NonDetect,
    Detecting
};

struct SpeechSensorStatus {
    SpeechStatus status;
    command_word_t command;
};


static constexpr const char *g_pConfigFilePath = "/spiffs/setting.json";

#endif //IOTSWITCH_COMMON_H
