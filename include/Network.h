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
    void update_data(int temp, int hum, int ph);
    bool get_kontrol_data();
    int get_set_point();
    String get_history_title();
    void append_suhu_to_history(int suhu, String title);
    void append_kelembaban_to_history(int kelembaban, String title);
    void update_time_history(String time, String title);
    void update_time(String time);
};

#endif // !Network_H_
