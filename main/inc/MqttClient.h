//
// Created by diam on 24-1-17.
//

#ifndef IOTSWITCH_MQTTCLIENT_H
#define IOTSWITCH_MQTTCLIENT_H

#include "Observer.h"
#include "esp_mqtt.hpp"
#include "esp_mqtt_client_config.hpp"


#include <memory>
#include "AppConfig.h"


enum MqttConnectionStatus {
    Init,
    Connected,
    Disconnected
};


class MqttClient : public idf::mqtt::Client {

private:

    /**
     * Switch Data Topic Filter
     */
    std::unique_ptr<idf::mqtt::Filter> dataFilterPtr;

    /**
     * Config Data Topic Filter
     */
    std::unique_ptr<idf::mqtt::Filter> configFilterPtr;

    /**
     * LiveData Status For Mqtt
     */
    LiveData<MqttConnectionStatus> connectionStatusLiveData;

protected:

    void on_disconnected(esp_mqtt_event_handle_t event) override;

    void on_connected(esp_mqtt_event_handle_t event) override;

    void on_data(esp_mqtt_event_handle_t event) override;

public:

    void publishStatus();

    LiveData<MqttConnectionStatus> &getConnectionStatusLiveData() {
        return connectionStatusLiveData;
    };

    // DO NOT USE THIS DIRECT!
    MqttClient(const idf::mqtt::BrokerConfiguration &broker, const idf::mqtt::ClientCredentials &credentials,
               const idf::mqtt::Configuration &config);

    ~MqttClient() override;
};

/**
 * Mqtt Manager
 */
class MqttClientManager {
public:

    /**
     * Build It
     * @return smart pointer
     */
    static std::shared_ptr<MqttClient> &build();

    static std::shared_ptr<MqttClient> &getClient();
};


#endif //IOTSWITCH_MQTTCLIENT_H
