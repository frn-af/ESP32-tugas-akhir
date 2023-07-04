#include "Network.h"
#include <addons/TokenHelper.h>

#define WIFI_SSID "Tenda"
#define WIFI_PASSWORD "d212blbr"
#define API_KEY "AIzaSyAQqn8sTOO34RnbW5AWfegClYKqe-XM8E4"
#define FIREBASE_PROJECT_ID "tugas-akhir-997ec"
#define USER_EMAIL "esp32@esp.com"
#define USER_PASSWORD "12345678"

static Network *instance = NULL;

Network::Network(/* args */)
{
    instance = this;
}

void WiFiEventConnected(WiFiEvent_t event, WiFiEventInfo_t info)
{
    Serial.println("Connected to wifi successfully!");
}

void WiFiEventDisconnected(WiFiEvent_t event, WiFiEventInfo_t info)
{
    Serial.println("Disconnected from WiFi access point!");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

void WiFiEventGotIP(WiFiEvent_t event, WiFiEventInfo_t info)
{
    Serial.println("Connected to the WiFi network!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
}

void Network::initWiFi()
{
    WiFi.onEvent(WiFiEventConnected, ARDUINO_EVENT_WIFI_STA_CONNECTED);
    WiFi.onEvent(WiFiEventDisconnected, ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
    WiFi.onEvent(WiFiEventGotIP, ARDUINO_EVENT_WIFI_STA_GOT_IP);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

void Network::FirebaseInit()
{
    config.api_key = API_KEY;
    auth.user.email = USER_EMAIL;
    auth.user.password = USER_PASSWORD;
    config.token_status_callback = tokenStatusCallback;
    Firebase.begin(&config, &auth);
}

void Network::DataUpdate(int temp, int hum)
{

    if (WiFi.status() == WL_CONNECTED && Firebase.ready())
    {
        String DocPath = "tools/monitoring";
        FirebaseJson content;

        content.set("fields/suhu/doubleValue", temp);
        content.set("fields/kelembaban/doubleValue", hum);

        if (Firebase.Firestore.patchDocument(&fbdo, FIREBASE_PROJECT_ID, "", DocPath.c_str(), content.raw(), "suhu,kelembaban"))
        {
            Serial.println("Patch Success");
        }
        else
        {
            Serial.println("Patch Failed");
            Serial.println(fbdo.errorReason());
        }
    }
}

bool Network::kontrolData()
{
    if (WiFi.status() == WL_CONNECTED && Firebase.ready())
    {
        String DocPath = "tools/kontrol";
        String mask = "kontrol";

        if (Firebase.Firestore.getDocument(&fbdo, FIREBASE_PROJECT_ID, "", DocPath.c_str(), mask.c_str()))
        {
            FirebaseJson payload;
            payload.setJsonData(fbdo.payload());
            FirebaseJsonData jsonData;
            payload.get(jsonData, "fields/kontrol/booleanValue");

            return jsonData.boolValue;
        }
        else
        {
            Serial.println("Get Failed");
            Serial.println(fbdo.errorReason());
            return false;
        }
    }
}
