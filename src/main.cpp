#include <Arduino.h>
#include "Network.h"
#include "SensorData.h"
#include <Ticker.h>
#include <LiquidCrystal_I2C.h>
#include <PID_v1.h>

const int PWM = 18;
const int RELAY = 26;
bool kontrol = false;
double setpoint, input, output, temp, hum, ph;
double Kp = 2, Ki = 5, Kd = 1;

Network *network;
SensorData *sensorData;
TaskHandle_t DHTtaskHandle = NULL;
Ticker DHTticker;
LiquidCrystal_I2C lcd = LiquidCrystal_I2C(0x27, 16, 2);
PID myPID(&input, &output, &setpoint, Kp, Ki, Kd, DIRECT);

void init_network();
void init_sensor_data();
void sensor_task(void *pvParameters);
void trigger_sensor_task();
void get_sensor_data();

void init_sensor_task()
{
  init_sensor_data();

  xTaskCreatePinnedToCore(
      sensor_task,    /* Task function. */
      "sensorTask",   /* String with name of task. */
      10000,          /* Stack size in bytes. */
      NULL,           /* Parameter passed as input of the task */
      1,              /* Priority of the task. */
      &DHTtaskHandle, /* Task handle. */
      1);             /* Core where the task should run */

  DHTticker.attach(5, trigger_sensor_task);
}

void trigger_sensor_task()
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
  lcd.init();
  lcd.backlight();
  pinMode(PWM, OUTPUT);
  pinMode(RELAY, OUTPUT);

  lcd.setCursor(0, 0);
  lcd.print("System starting...");

  init_network();

  lcd.setCursor(0, 0);
  lcd.println("Network ready");
  lcd.setCursor(0, 1);
  lcd.println("initiating");

  init_sensor_task();
}

void loop()
{
  if (WiFi.status() == WL_CONNECTED && Firebase.ready())
  {
    delay(2000);

    kontrol = network->get_kontrol_data();
    setpoint = network->get_set_point();
    Serial.println("kontrol :" + String(kontrol));

    if (kontrol == true)
    {
      Serial.println("DHTtask resumed");
      vTaskResume(DHTtaskHandle);
      input = temp;
      myPID.Compute();
      Serial.println("PID: " + String(output));

      if (output > 0)
      {
        digitalWrite(RELAY, HIGH);
      }
      else
      {
        digitalWrite(RELAY, LOW);
      }

      analogWrite(PWM, (int)output);
    }
    else
    {
      Serial.println("DHTtask suspended");
      vTaskSuspend(DHTtaskHandle);
    }
  }
  yield();
}

void sensor_task(void *pvParameters)
{
  for (;;)
  {
    get_sensor_data();
  }
}

void get_sensor_data()
{
  TempAndHumidity dhtData = sensorData->get_dht_data();
  temp = dhtData.temperature;
  hum = dhtData.humidity;
  ph = sensorData->get_ph_data();

  Serial.println("Temp: " + String(temp));
  Serial.println("Hum: " + String(hum));
  Serial.println("PH: " + String(ph));

  lcd.setCursor(0, 0);
  lcd.println("Temp: " + String(temp));
  lcd.setCursor(0, 1);
  lcd.println("Hum: " + String(hum));

  network->update_data(temp, hum, ph);
}

void init_sensor_data()
{
  sensorData = new SensorData();
  sensorData->init_dht();
  sensorData->init_ph();
}

void init_network()
{
  network = new Network();
  network->init_wifi();
  network->init_firebase();
}