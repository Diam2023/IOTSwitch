//
// Created by diam on 24-1-17.
//

#include "app_switch.h"


using namespace idf;

AppSwitch::AppSwitch(const mqtt::BrokerConfiguration &broker, const mqtt::ClientCredentials &credentials,
                     const mqtt::Configuration &config) : Client(broker, credentials, config) {
    switchStatus.append([this](auto &status){

    });
}

void AppSwitch::publishStatus() {

}
