#include <Arduino.h>
#include "Network.h"
#include "SensorData.h"
#include <Ticker.h>

Network *network;
SensorData *sensorData;

TaskHandle_t DHTtaskHandle = NULL;

Ticker DHTticker;

void initNetwork();
void initSensorData();
void DHTtask(void *pvParameters);
void getDHTData();

void initDHT()
{
  initSensorData();

  xTaskCreate(
      DHTtask,         /* Task function. */
      "DHTtask",       /* String with name of task. */
      10000,           /* Stack size in bytes. */
      NULL,            /* Parameter passed as input of the task */
      1,               /* Priority of the task. */
      &DHTtaskHandle); /* Task handle. */
}

void setup()
{
  Serial.begin(115200);
  Serial.println("System starting...");
  initNetwork();
  initDHT();
}

void loop()
{
  if (WiFi.status() == WL_CONNECTED && Firebase.ready())
  {
    delay(5000);

    bool kontrol = network->kontrolData();
    Serial.println("kontrol :" + String(kontrol));

    if (DHTtaskHandle != NULL && kontrol == true)
    {
      vTaskResume(DHTtaskHandle);
    }
  }
  else
  {
    if (DHTtaskHandle != NULL)
    {
      vTaskSuspend(DHTtaskHandle);
    }
  }
  yield();
}

void getDHTData()
{
  TempAndHumidity data = sensorData->getDHTData();
  // data temperature and humidity to number

  Serial.println("Temperature: " + String(data.temperature) + " C");
  Serial.println("Humidity: " + String(data.humidity) + " %");
  int temp = data.temperature;
  int hum = data.humidity;
  network->DataUpdate(temp, hum);
}

void DHTtask(void *pvParameters)
{
  for (;;)
  {
    getDHTData();
    vTaskDelay(10000 / portTICK_PERIOD_MS);
  }
}

void initSensorData()
{
  sensorData = new SensorData();
  sensorData->initDHT();
}

void initNetwork()
{
  network = new Network();
  network->initWiFi();
  network->FirebaseInit();
}