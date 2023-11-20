/**
 * Created by Farhan Alfathra (frn.af)
 *
 * Github: https://github.com/frn-af/esp32-fermentation
 */
#include "Network.h"

// Provide the token generation process info.
#include <addons/TokenHelper.h>

/* 1. Define the WiFi credentials */
#define WIFI_SSID "XXXXXX"
#define WIFI_PASSWORD "XXXXXXXX"

/* 2. Define the API Key */
#define API_KEY "XXXXXXXXXXXXXXXX"

/* 3. Define the project ID */
#define FIREBASE_PROJECT_ID "XXXXXXXXXXX"

/* 4. Define the user Email and password that alreadey registerd or added in your project */
#define USER_EMAIL "XXXXXXXXXXXXX"
#define USER_PASSWORD "XXXXXXXXXXXX"

static Network *instance = NULL;

Network::Network(/* args */)
{
    instance = this;
}

/*
 *  Callback function to get the token status from Firebase.
 * The event will be triggered when the token is expired.
 * @param status The token status string from Firebase.
 * @param data The additional data string that defined in the TokenHelper.h.
 */
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
/* initialized wifi network with ssid and password
 * event handler for wifi connection
 * so the connection will be more stable
 */
void Network::init_wifi()
{

    WiFi.onEvent(WiFiEventConnected, ARDUINO_EVENT_WIFI_STA_CONNECTED);
    WiFi.onEvent(WiFiEventDisconnected, ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
    WiFi.onEvent(WiFiEventGotIP, ARDUINO_EVENT_WIFI_STA_GOT_IP);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}
/*
 * initialized firebase with api key, user email, and user password
 * and also token status callback
 */
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

    Firebase.begin(&config, &auth);
    Firebase.reconnectWiFi(true);
}

/*
 * in this section below is the function for get and update data from firebase
 * all function is using firebase firestore
 * for more information about firestore, see https://firebase.google.com/docs/firestore
 * or check documentation in https://github.com/mobizt/Firebase-ESP-Client/tree/main
 */
bool Network::get_kontrol_data()
{
    if (WiFi.status() == WL_CONNECTED && Firebase.ready())
    {
        String documentPath = "tools/monitoring";
        String mask = "kontrol";

        // If the document path contains space e.g. "a b c/d e f"
        // It should encode the space as %20 then the path will be "a%20b%20c/d%20e%20f"

        Serial.print("Get a document... ");

        if (Firebase.Firestore.getDocument(&fbdo, FIREBASE_PROJECT_ID, "", documentPath.c_str(), mask.c_str()))
        {

            FirebaseJson payload;
            payload.setJsonData(fbdo.payload());
            FirebaseJsonData jsonData;
            payload.get(jsonData, "fields/kontrol/booleanValue");

            if (jsonData.success)
            {
                Serial.println(jsonData.boolValue ? "true" : "false");
                return jsonData.boolValue;
            }
        }
        else
            Serial.println(fbdo.errorReason());
        return false;
    }
    else
    {
        Serial.println("******************************");
        Serial.println("wifi not connected or firebase not ready");
        return false;
    }
}

void Network::update_data(int temp, int hum, double ph)
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
        Serial.println("******************************");
        Serial.println("wifi not connected or firebase not ready");
        return 0;
    }
}

String Network::get_history_title()
{
    if (WiFi.status() == WL_CONNECTED && Firebase.ready())
    {
        String DocPath = "tools/monitoring";
        String mask = "history";

        if (Firebase.Firestore.getDocument(&fbdo, FIREBASE_PROJECT_ID, "", DocPath.c_str(), mask.c_str()))
        {
            FirebaseJson payload;
            payload.setJsonData(fbdo.payload());
            FirebaseJsonData jsonData;
            payload.get(jsonData, "fields/history/stringValue");

            return jsonData.stringValue;
        }
        else
        {
            Serial.println("******************************");
            Serial.println("failed get history title");
            Serial.println(fbdo.errorReason());
            Serial.println("******************************");
            return "";
        }
    }
    else
    {
        Serial.println("******************************");
        Serial.println("wifi not connected or firebase not ready");
        return "";
    }
}

void Network::append_suhu_to_history(int suhu, String title)
{
    std::vector<struct fb_esp_firestore_document_write_t> writes;

    // A write object that will be written to the document.
    struct fb_esp_firestore_document_write_t transform_write;

    // Set the write object write operation type.
    // fb_esp_firestore_document_write_type_update,
    // fb_esp_firestore_document_write_type_delete,
    // fb_esp_firestore_document_write_type_transform
    transform_write.type = fb_esp_firestore_document_write_type_transform;

    // Set the document path of document to write (transform)
    transform_write.document_transform.transform_document_path = "history/" + title;

    // Set a transformation of a field of the document.
    struct fb_esp_firestore_document_write_field_transforms_t field_transforms;

    // Set field path to write.
    field_transforms.fieldPath = "suhu";

    // Set the transformation type.
    // fb_esp_firestore_transform_type_set_to_server_value,
    // fb_esp_firestore_transform_type_increment,
    // fb_esp_firestore_transform_type_maaximum,
    // fb_esp_firestore_transform_type_minimum,
    // fb_esp_firestore_transform_type_append_missing_elements,
    // fb_esp_firestore_transform_type_remove_all_from_array
    field_transforms.transform_type = fb_esp_firestore_transform_type_append_missing_elements;

    // For the usage of FirebaseJson, see examples/FirebaseJson/BasicUsage/Create_Edit_Parse/Create_Edit_Parse.ino
    FirebaseJson content;

    content.set("values/[0]/integerValue", suhu);

    // Set the transformation content.
    field_transforms.transform_content = content.raw();

    // Add a field transformation object to a write object.
    transform_write.document_transform.field_transforms.push_back(field_transforms);

    // Add a write object to a write array.
    writes.push_back(transform_write);

    if (Firebase.Firestore.commitDocument(&fbdo, FIREBASE_PROJECT_ID, "" /* databaseId can be (default) or empty */, writes /* dynamic array of fb_esp_firestore_document_write_t */, "" /* transaction */))
        Serial.printf("ok\n%s\n\n", fbdo.payload().c_str());
    else
        Serial.println(fbdo.errorReason());
}

void Network::append_kelembaban_to_history(int kelembaban, String title)
{

    std::vector<struct fb_esp_firestore_document_write_t> writes;

    // A write object that will be written to the document.
    struct fb_esp_firestore_document_write_t transform_write;

    // Set the write object write operation type.
    // fb_esp_firestore_document_write_type_update,
    // fb_esp_firestore_document_write_type_delete,
    // fb_esp_firestore_document_write_type_transform
    transform_write.type = fb_esp_firestore_document_write_type_transform;

    // Set the document path of document to write (transform)
    transform_write.document_transform.transform_document_path = "history/" + title;

    // Set a transformation of a field of the document.
    struct fb_esp_firestore_document_write_field_transforms_t field_transforms;

    // Set field path to write.
    field_transforms.fieldPath = "kelembaban";

    // Set the transformation type.
    // fb_esp_firestore_transform_type_set_to_server_value,
    // fb_esp_firestore_transform_type_increment,
    // fb_esp_firestore_transform_type_maaximum,
    // fb_esp_firestore_transform_type_minimum,
    // fb_esp_firestore_transform_type_append_missing_elements,
    // fb_esp_firestore_transform_type_remove_all_from_array
    field_transforms.transform_type = fb_esp_firestore_transform_type_append_missing_elements;

    // For the usage of FirebaseJson, see examples/FirebaseJson/BasicUsage/Create_Edit_Parse/Create_Edit_Parse.ino
    FirebaseJson content;

    content.set("values/[0]/integerValue", kelembaban);

    // Set the transformation content.
    field_transforms.transform_content = content.raw();

    // Add a field transformation object to a write object.
    transform_write.document_transform.field_transforms.push_back(field_transforms);

    // Add a write object to a write array.
    writes.push_back(transform_write);

    if (Firebase.Firestore.commitDocument(&fbdo, FIREBASE_PROJECT_ID, "" /* databaseId can be (default) or empty */, writes /* dynamic array of fb_esp_firestore_document_write_t */, "" /* transaction */))
        Serial.printf("ok\n%s\n\n", fbdo.payload().c_str());
    else
        Serial.println(fbdo.errorReason());
}

void Network::update_time(String time)
{

    if (WiFi.status() == WL_CONNECTED && Firebase.ready())
    {
        String DocPath = "tools/time";
        FirebaseJson content;

        content.set("fields/time/stringValue", time);

        if (Firebase.Firestore.patchDocument(&fbdo, FIREBASE_PROJECT_ID, "", DocPath.c_str(), content.raw(), "time"))
        {
            Serial.println("******************************");
            Serial.println("update time success");
            Serial.println("******************************");
        }
        else
        {
            Serial.println("******************************");
            Serial.println("failed update time");
            Serial.println(fbdo.errorReason());
            Serial.println("******************************");
        }
    }
}

void Network::update_time_history(String time, String title)
{
    if (WiFi.status() == WL_CONNECTED && Firebase.ready())
    {
        String DocPath = "history/" + title;
        FirebaseJson content;

        content.set("fields/lama_fermentasi/stringValue", time);

        if (Firebase.Firestore.patchDocument(&fbdo, FIREBASE_PROJECT_ID, "", DocPath.c_str(), content.raw(), "lama_fermentasi"))
        {
            Serial.println("******************************");
            Serial.println("update history success");
            Serial.println("******************************");
        }
        else
        {
            Serial.println("******************************");
            Serial.println("failed update history");
            Serial.println(fbdo.errorReason());
            Serial.println("******************************");
        }
    }
}

void Network::notification(String date)
{
    if (WiFi.status() == WL_CONNECTED)
    {
        HTTPClient http;

        // URL of the API endpoint
        const char *url = "https://app.nativenotify.com/api/notification";

        // JSON body data
        String jsonBody = "{\"appId\": 9849, \"appToken\": \"MLENeeUtxrJE0rYHESEHYO\", \"title\": \"Tapai Telah Matang\", \"body\": \"Proses fermentasi telah selesai, Harap matikan system\", \"dateSent\": \"" + date + "\"}";

        http.begin(url);

        // Set headers
        http.addHeader("Content-Type", "application/json");

        // Send POST request and get response
        int httpResponseCode = http.POST(jsonBody);

        // Check for response
        if (httpResponseCode > 0)
        {
            Serial.print("HTTP Response Code: ");
            Serial.println(httpResponseCode);
            String response = http.getString();
            Serial.println(response);
        }
        else
        {
            Serial.print("Error sending POST request. HTTP Response Code: ");
            Serial.println(httpResponseCode);
        }

        http.end();
    }
}
