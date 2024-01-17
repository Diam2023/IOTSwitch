#pragma once

#include "Observer.h"
#include "Common.h"

#include <thread>
#include <deque>

struct BeepTask {
    int count;
    int timeDelay;
};

class AppBuzzer : public Listener<SwitchStatus> {
private:
    int targetPin;

    std::shared_ptr<std::thread> beepThread;

    std::shared_ptr<std::deque<BeepTask>> beepTasks;
public:

    /**
     * Beep Task
     */
    [[noreturn]] void beepTask();

    /**
     * switch listener
     * @param d switch status
     */
    void onListener(const SwitchStatus &d) override;

    AppBuzzer();

    void beepMicroTimeAsync(BeepTask &&t);

    void beepMicroTimeSync(const BeepTask &t);

    static AppBuzzer &getInstance() {
        static AppBuzzer buzzer;
        return buzzer;
    }
};
