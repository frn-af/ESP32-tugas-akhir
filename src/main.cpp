#include <Arduino.h>
#include "Network.h"
#include "SensorData.h"
#include <Ticker.h>
#include <LiquidCrystal_I2C.h>
#include <PID_v1.h>

#define NTP_SERVER "pool.ntp.org"
#define UTC_OFFSET 25200
#define DST_OFFSET 0

/*
 * define pin for hardware
 * pin for DHT, PH declare here so we dont need to change the lib
 */
const int PWM = 18;
const int RELAY = 26;
const int DHTPin = 19;
const int pHPin = 32;

bool kontrol = false;
double temp, hum, ph;

double Setpoint, Input, Output;
double Kp = 2, Ki = 5, Kd = 1;

Network *network;
SensorData *sensorData;
LiquidCrystal_I2C lcd = LiquidCrystal_I2C(0x27, 16, 2);
PID myPID(&Input, &Output, &Setpoint, Kp, Ki, Kd, DIRECT);

Ticker ticker;
TaskHandle_t sensorTask;

void init_network();
void init_sensor_data();
void get_sensor_data();
void get_time_info();
void init_sensor_task();
void sensor_task(void *pvParameters);

void setup()
{
  Serial.begin(115200);
  Serial.println("System starting...");
  lcd.init();
  lcd.backlight();
  pinMode(PWM, OUTPUT);
  pinMode(RELAY, OUTPUT);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.println("System starting.");

  init_network();

  lcd.setCursor(0, 0);
  lcd.println("Network ready...");
  lcd.setCursor(0, 1);
  lcd.println("init system.....");

  myPID.SetMode(AUTOMATIC);

  configTime(UTC_OFFSET, DST_OFFSET, NTP_SERVER);

  kontrol = network->get_kontrol_data();

  init_sensor_task();
}

void loop()
{

  if (WiFi.status() == WL_CONNECTED && Firebase.ready())
  {
    kontrol = network->get_kontrol_data();
    Setpoint = network->get_set_point();

    Serial.println("******************************");
    Serial.println("Kontrol: " + String(kontrol));
    Serial.println("Setpoint: " + String(Setpoint));
    Serial.println(" ");
    delay(3000);
  }

  if (kontrol == true)
  {
    delay(1000);
    vTaskResume(sensorTask);
  }

  if (kontrol == false)
  {
    vTaskSuspend(sensorTask);
    Serial.println("******************************");
    Serial.println("system paused");

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.println("  waiting for   ");
    lcd.setCursor(0, 1);
    lcd.println(" actived system ");

    delay(1000);
  }
}

void init_sensor_task()
{
  init_sensor_data();

  xTaskCreate(
      sensor_task,   /* Function to implement the task */
      "sensor_task", /* Name of the task */
      10000,         /* Stack size in words */
      NULL,          /* Task input parameter */
      1,             /* Priority of the task */
      &sensorTask);  /* Task handle. */
}

void sensor_task(void *pvParameters)
{
  for (;;)
  {
    get_sensor_data();
    get_time_info();
    vTaskDelay(5000 / portTICK_PERIOD_MS);
  }
}

void get_sensor_data()
{
  TempAndHumidity dhtData = sensorData->get_dht_data();
  temp = dhtData.temperature;
  hum = dhtData.humidity;
  ph = sensorData->get_ph_data();

  Input = temp;
  myPID.Compute();

  analogWrite(PWM, (int)Output);

  if (Input > Setpoint)
  {
    digitalWrite(RELAY, LOW);
  }
  else
  {
    digitalWrite(RELAY, HIGH);
  }

  Serial.println("******************************");
  Serial.println("Temp: " + String(temp));
  Serial.println("Hum: " + String(hum));
  Serial.println("PH: " + String(ph));
  Serial.println(" ");

  Serial.println("******************************");
  Serial.println("Input: " + String(Input));
  Serial.println("Output: " + String(Output));
  Serial.println(" ");

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.println("Temp:" + String((int)temp));
  lcd.setCursor(0, 1);
  lcd.println("Hum :" + String((int)hum));

  network->update_data(temp, hum, ph);
}

void get_time_info()
{
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo))
  {
    Serial.println("******************************");
    Serial.println("Failed to obtain time");
    Serial.println(" ");
    return;
  }

  String date = String(timeinfo.tm_mday) + "/" + String(timeinfo.tm_mon + 1) + "/" + String(timeinfo.tm_year + 1900);

  Serial.println("******************************");
  Serial.println(date);
  Serial.println(" ");

  lcd.setCursor(8, 1);
  lcd.println(&timeinfo, "%d/%m/%y");
}

void init_sensor_data()
{
  sensorData = new SensorData(DHTPin, pHPin);
  sensorData->init_dht();
  sensorData->init_ph();
}

void init_network()
{
  network = new Network();
  network->init_wifi();
  network->init_firebase();
}