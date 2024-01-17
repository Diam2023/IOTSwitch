//
// Created by diam on 24-1-17.
//

#ifndef IOTSWITCH_MQTTCLIENT_H
#define IOTSWITCH_MQTTCLIENT_H

#include "Observer.h"
#include "esp_mqtt.hpp"
#include "esp_mqtt_client_config.hpp"

#define DEVICE_SN "ss81kq"


class MqttClient : public idf::mqtt::Client {

private:
    // TODO SN
    idf::mqtt::Filter data{"$SYS/devices/" DEVICE_SN "/data"};
    idf::mqtt::Filter config{"$SYS/devices/" DEVICE_SN "/config"};

protected:
    void on_disconnected(esp_mqtt_event_handle_t event) override;

    void on_connected(esp_mqtt_event_handle_t event) override;

    void on_data(esp_mqtt_event_handle_t event) override;
};


#endif //IOTSWITCH_MQTTCLIENT_H
