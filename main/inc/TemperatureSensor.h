//
// Created by diam on 24-1-17.
//

#ifndef IOTSWITCH_TEMPERATURESENSORE_H
#define IOTSWITCH_TEMPERATURESENSORE_H

#include "Observer.h"
#include "VoltageSensor.h"

class TemperatureSensor : public Listener<uint32_t> {
private:
    uint32_t temperatureThreshold;

public:
    TemperatureSensor();

    void onListener(const unsigned long &d) override;

    static TemperatureSensor &getInstance() {
        static TemperatureSensor temperatureSensor;
        return temperatureSensor;
    }
};


#endif //IOTSWITCH_TEMPERATURESENSORE_H
