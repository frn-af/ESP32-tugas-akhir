#ifndef Network_H_
#define Network_H_

#include <WiFi.h>
#include <Firebase_ESP_Client.h>

class Network
{
private:
    /* data */
    FirebaseData fbdo;
    FirebaseAuth auth;
    FirebaseConfig config;

    friend void WiFiEventConnected(WiFiEvent_t event, WiFiEventInfo_t info);
    friend void WiFiEventDisconnected(WiFiEvent_t event, WiFiEventInfo_t info);
    friend void WiFiEventGotIP(WiFiEvent_t event, WiFiEventInfo_t info);

public:
    Network(/* args */);
    void init_wifi();
    void init_firebase();
    void update_data(double temp, double hum, double ph);
    bool get_kontrol_data();
    int get_set_point();
};

#endif // !Network_H_
