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
    friend void FirestoreTokenStatusCallback(TokenInfo info);

public:
    Network(/* args */);
    void initWiFi();
    void FirebaseInit();
    void DataUpdate(double temp, double hum, double ph);
    bool kontrolData();
};

#endif // !Network_H_
