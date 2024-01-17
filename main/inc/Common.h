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


static constexpr const char *g_pConfigFilePath = "/spiffs/setting.json";

#endif //IOTSWITCH_COMMON_H
