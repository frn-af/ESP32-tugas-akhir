#include "Network.h"
// Provide the token generation process info.
#include <addons/TokenHelper.h>

/* 1. Define the WiFi credentials */
#define WIFI_SSID "Tenda"
#define WIFI_PASSWORD "d212blbr"

/* 2. Define the API Key */
#define API_KEY "AIzaSyAQqn8sTOO34RnbW5AWfegClYKqe-XM8E4"

/* 3. Define the project ID */
#define FIREBASE_PROJECT_ID "tugas-akhir-997ec"

/* 4. Define the user Email and password that alreadey registerd or added in your project */
#define USER_EMAIL "esp32@esp.com"
#define USER_PASSWORD "12345678"

static Network *instance = NULL;

Network::Network(/* args */)
{
    instance = this;
}

void WiFiEventConnected(WiFiEvent_t event, WiFiEventInfo_t info)
{
    Serial.println("******************************");
    Serial.println("Connected to wifi successfully!");
    Serial.println("******************************");
}

void WiFiEventDisconnected(WiFiEvent_t event, WiFiEventInfo_t info)
{
    Serial.println("******************************");
    Serial.println("Disconnected from WiFi access point!");
    Serial.println("******************************");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

void WiFiEventGotIP(WiFiEvent_t event, WiFiEventInfo_t info)
{
    Serial.println("******************************");
    Serial.println("Connected to the WiFi network!");
    Serial.print("IP Address: ");
    Serial.println("******************************");
    Serial.println(WiFi.localIP());
}

void Network::init_wifi()
{

    WiFi.onEvent(WiFiEventConnected, ARDUINO_EVENT_WIFI_STA_CONNECTED);
    WiFi.onEvent(WiFiEventDisconnected, ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
    WiFi.onEvent(WiFiEventGotIP, ARDUINO_EVENT_WIFI_STA_GOT_IP);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

void Network::init_firebase()
{

    Serial.println("*** Firebase Client - Firestore ***");
    Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

    /* Assign the api key (required) */
    config.api_key = API_KEY;

    /* Assign the user sign in credentials */
    auth.user.email = USER_EMAIL;
    auth.user.password = USER_PASSWORD;

    /* Assign the callback function for the long running token generation task */
    config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h

    // Limit the size of response payload to be collected in FirebaseData
    fbdo.setResponseSize(2048);

    Firebase.begin(&config, &auth);

    Firebase.reconnectWiFi(true);
}

bool Network::get_kontrol_data()
{
    if (WiFi.status() == WL_CONNECTED && Firebase.ready())
    {
        String DocPath = "tools/monitoring";
        String mask = "kontrol";

        if (Firebase.Firestore.getDocument(&fbdo, FIREBASE_PROJECT_ID, "", DocPath.c_str(), mask.c_str()))
        {
            FirebaseJson payload;
            payload.setJsonData(fbdo.payload());
            FirebaseJsonData jsonData;
            payload.get(jsonData, "fields/kontrol/booleanValue");
            if (jsonData.boolValue == 1)
            {
                return true;
            }
            else
            {
                return false;
            }
        }
        else
        {
            Serial.println("******************************");
            Serial.println("failed get kontrol data");
            Serial.println(fbdo.errorReason());
            Serial.println("******************************");
            return false;
        }
    }
    else
    {
        return false;
    }
}

void Network::update_data(double temp, double hum, double ph)
{

    if (WiFi.status() == WL_CONNECTED && Firebase.ready())
    {
        String DocPath = "tools/monitoring";
        FirebaseJson content;

        content.set("fields/suhu/doubleValue", temp);
        content.set("fields/kelembaban/doubleValue", hum);
        content.set("fields/ph/doubleValue", ph);

        if (Firebase.Firestore.patchDocument(&fbdo, FIREBASE_PROJECT_ID, "", DocPath.c_str(), content.raw(), "suhu,kelembaban,ph"))
        {
            Serial.println("******************************");
            Serial.println("update data success");
            Serial.println("******************************");
        }
        else
        {
            Serial.println("******************************");
            Serial.println("failed update data");
            Serial.println(fbdo.errorReason());
            Serial.println("******************************");
        }
    }
}

int Network::get_set_point()
{
    if (WiFi.status() == WL_CONNECTED && Firebase.ready())
    {
        String DocPath = "tools/monitoring";
        String mask = "setpoint";

        if (Firebase.Firestore.getDocument(&fbdo, FIREBASE_PROJECT_ID, "", DocPath.c_str(), mask.c_str()))
        {
            FirebaseJson payload;
            payload.setJsonData(fbdo.payload());
            FirebaseJsonData jsonData;
            payload.get(jsonData, "fields/setpoint/integerValue");

            return jsonData.intValue;
        }
        else
        {
            Serial.println("******************************");
            Serial.println("failed get setpoint");
            Serial.println(fbdo.errorReason());
            Serial.println("******************************");
            return 0;
        }
    }
    else
    {
        return 0;
    }
}
