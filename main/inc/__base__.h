#pragma once

#include <list>

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "freertos/semphr.h"


typedef enum
{
    COMMAND_TIMEOUT = -2,
    COMMAND_NOT_DETECTED = -1,

    MENU_REBOOT = 0,
    MENU_RED = 1,
    MENU_BLUE = 2,
    MENU_GREEN = 3,
    MENU_ON = 4,
    MENU_OFF = 5,
} command_word_t;

class Observer
{
public:
    virtual void update() = 0;
};

class Subject
{
private:
    std::list<Observer *> observers;

public:
    void attach(Observer *observer)
    {
        this->observers.push_back(observer);
    }

    void detach(Observer *observer)
    {
        this->observers.remove(observer);
    }

    void detach_all()
    {
        this->observers.clear();
    }

    void notify()
    {
        for (auto observer : this->observers)
            observer->update();
    }
};

