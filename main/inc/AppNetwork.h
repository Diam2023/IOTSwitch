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
    ConnectionError,
    Disconnected,
    TimeOut
};

class AppNetwork {

private:
    std::string ssid;
    std::string pwd;

    LiveData<NetworkStatus> networkStatusLiveData;

protected:

    /**
     * Async
     */
    bool connect();

public:

    auto &getNetworkStatusLiveData() {
        return networkStatusLiveData;
    }

    bool start();

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
