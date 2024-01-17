//
// Created by diam on 24-1-1.
//
#pragma once

#include "Observer.h"
#include "Common.h"

#include <thread>

using namespace std::chrono_literals;


class TouchpadSensor {

private:
    int64_t notifyThreshold;
    std::chrono::microseconds touchLimitThreshold;

    LiveData<TouchStatus> touchStatus;

public:
    explicit TouchpadSensor(int64_t notifyThreshold_ = 100000, std::chrono::microseconds touchLimitThreshold_ = 500ms);

    void run();

    void setTouchStatus(const TouchStatus &status) {
        touchStatus = status;
    }

    [[nodiscard]] TouchStatus getTouchStatus() {
        return (*touchStatus);
    }

    LiveData<TouchStatus> &getTouchStatusLiveData() {
        return touchStatus;
    }

    [[nodiscard]] int64_t getNotifyThreshold() const {
        return notifyThreshold;
    }

    void setNotifyThreshold(int64_t notifyThreshold_) {
        TouchpadSensor::notifyThreshold = notifyThreshold_;
    }

    [[nodiscard]] const std::chrono::microseconds &getTouchLimitThreshold() const {
        return touchLimitThreshold;
    }

    void setTouchLimitThreshold(const std::chrono::microseconds &touchLimitThreshold_) {
        TouchpadSensor::touchLimitThreshold = touchLimitThreshold_;
    }

};

