//
// Created by diam on 24-1-17.
//

#ifndef IOTSWITCH_APPRELAY_H
#define IOTSWITCH_APPRELAY_H

#include "Common.h"
#include "Observer.h"

#include "AppConfig.h"
#include "AppSwitch.h"

#include <gpio_cxx.hpp>


class AppRelay : public Listener<SwitchStatus> {

private:
    idf::GPIO_Output targetGpio;

public:
    void onListener(const SwitchStatus &d) override;

    AppRelay();

    static AppRelay& getInstance()
    {
        static AppRelay relay;
        return relay;
    }
};


#endif //IOTSWITCH_APPRELAY_H
