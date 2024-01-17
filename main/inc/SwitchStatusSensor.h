//
// Created by diam on 24-1-17.
//

#ifndef IOTSWITCH_SWITCHSTATUSSENSOR_H
#define IOTSWITCH_SWITCHSTATUSSENSOR_H

#include "VoltageSensor.h"

// This Sensor Use to recognize there is curr running in output
// That mean another switch in worked
// Publisher
// TODO Adc
class SwitchStatusSensor {

private:

protected:

public:

    SwitchStatusSensor();

    SwitchStatusSensor& getInstance()
    {
        static SwitchStatusSensor switchStatusSensor;
        return switchStatusSensor;
    }
};


#endif //IOTSWITCH_SWITCHSTATUSSENSOR_H
