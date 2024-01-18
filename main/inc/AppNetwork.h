//
// Created by diam on 24-1-18.
//

#ifndef IOTSWITCH_APPNETWORK_H
#define IOTSWITCH_APPNETWORK_H

#include <future>
#include <esp_wifi_netif.h>

#include "Observer.h"

enum NetworkStatus {
    ProvError,
    Connected,
    NotFound,
    ConnectionError,
    Disconnected,
    TimeOut
};

class AppNetwork {

private:
    std::string ssid;
    std::string pwd;

    LiveData<NetworkStatus> networkStatusLiveData;

public:

    static void asyncStart();

    static void syncStart();

//    static bool run();
    /**
     * Async
     */
    bool connect();

    auto &getNetworkStatusLiveData() {
        return networkStatusLiveData;
    }


    AppNetwork();

    static AppNetwork &getInstance() {
        static AppNetwork appNetwork;

        return appNetwork;
    }

    const std::string &getSsid() const {
        return ssid;
    }

    void setSsid(const std::string &ssid_) {
        AppNetwork::ssid = ssid_;
    }

    const std::string &getPwd() const {
        return pwd;
    }

    void setPwd(const std::string &pwd_) {
        AppNetwork::pwd = pwd_;
    }

};


#endif //IOTSWITCH_APPNETWORK_H
