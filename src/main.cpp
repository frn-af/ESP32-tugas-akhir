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
void sensorTask(void *pvParameters);
void triggerSensorTask();
void getSensordata();

bool kontrol = false;

void initDHT()
{
  initSensorData();

  xTaskCreatePinnedToCore(
      sensorTask,     /* Task function. */
      "sensorTask",   /* String with name of task. */
      10000,          /* Stack size in bytes. */
      NULL,           /* Parameter passed as input of the task */
      1,              /* Priority of the task. */
      &DHTtaskHandle, /* Task handle. */
      1);             /* Core where the task should run */

  DHTticker.attach(5, triggerSensorTask);
}

void triggerSensorTask()
{
  if (DHTtaskHandle != NULL)
  {
    xTaskResumeFromISR(DHTtaskHandle);
  }
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
    delay(2000);

    kontrol = network->kontrolData();
    Serial.println("kontrol :" + String(kontrol));

    if (kontrol == true)
    {
      Serial.println("DHTtask resumed");
      vTaskResume(DHTtaskHandle);
    }
    else
    {
      Serial.println("DHTtask suspended");
      vTaskSuspend(DHTtaskHandle);
    }
  }
  yield();
}

void sensorTask(void *pvParameters)
{
  for (;;)
  {
    getSensordata();
  }
}

void getSensordata()
{
  TempAndHumidity dhtData = sensorData->getDHTData();
  double temp = dhtData.temperature;
  double hum = dhtData.humidity;
  double ph = sensorData->getPHData();

  Serial.println("Temp: " + String(temp));
  Serial.println("Hum: " + String(hum));
  Serial.println("PH: " + String(ph));

  network->DataUpdate(temp, hum, ph);
}

void initSensorData()
{
  sensorData = new SensorData();
  sensorData->initDHT();
  sensorData->initPH();
}

void initNetwork()
{
  network = new Network();
  network->initWiFi();
  network->FirebaseInit();
}