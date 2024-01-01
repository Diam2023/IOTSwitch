//
// Created by diam on 24-1-1.
//
#pragma once

#include "__base__.h"

#include <thread>


using namespace std::chrono_literals;


class AppTouch : public Subject {

private:
    int64_t notifyThreshold;
    std::chrono::microseconds touchLimitThreshold;

public:
    explicit AppTouch(int64_t notifyThreshold_ = 100000, std::chrono::microseconds touchLimitThreshold_ = 500ms);

    void run();


    [[nodiscard]] int64_t getNotifyThreshold() const {
        return notifyThreshold;
    }

    void setNotifyThreshold(int64_t notifyThreshold_) {
        AppTouch::notifyThreshold = notifyThreshold_;
    }

    [[nodiscard]] const std::chrono::microseconds &getTouchLimitThreshold() const {
        return touchLimitThreshold;
    }

    void setTouchLimitThreshold(const std::chrono::microseconds &touchLimitThreshold_) {
        AppTouch::touchLimitThreshold = touchLimitThreshold_;
    }

};

