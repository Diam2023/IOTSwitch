//
// Created by diam on 24-1-17.
//

#ifndef IOTSWITCH_SWITCHSTATUSSENSOR_H
#define IOTSWITCH_SWITCHSTATUSSENSOR_H

#include "VoltageSensor.h"

#include "Observer.h"

// This Sensor Use to recognize there is curr running in output
// That mean another switch in worked
// Publisher
// TODO Adc
class SwitchStatusSensor : public Listener<uint32_t> {

private:
    uint32_t outputThreshold;

protected:

public:

    void onListener(const unsigned long &d) override;

    SwitchStatusSensor();

    static SwitchStatusSensor &getInstance() {
        static SwitchStatusSensor switchStatusSensor;
        return switchStatusSensor;
    }
};


#endif //IOTSWITCH_SWITCHSTATUSSENSOR_H
