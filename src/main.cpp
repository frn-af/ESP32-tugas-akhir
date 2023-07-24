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

unsigned long startTime, dataMillis, currentTime;

Network *network;
SensorData *sensorData;
LiquidCrystal_I2C lcd = LiquidCrystal_I2C(0x27, 16, 2);
PID myPID(&Input, &Output, &Setpoint, Kp, Ki, Kd, DIRECT);

void init_network();
void init_sensor_data();
void get_sensor_data();
void get_time_info();

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

  init_sensor_data();

  startTime = millis();
}

void loop()
{

  if (WiFi.status() == WL_CONNECTED && Firebase.ready() && (millis() - dataMillis > 5000 || dataMillis == 0))
  {
    dataMillis = millis();

    kontrol = network->get_kontrol_data();
    Setpoint = network->get_set_point();

    Serial.println("******************************");
    Serial.println("Kontrol: " + String(kontrol));
    Serial.println("Setpoint: " + String(Setpoint));
    Serial.println(" ");

    if (kontrol == true)
    {
      currentTime = millis() - startTime;

      unsigned long seconds = currentTime / 1000;
      unsigned long minutes = seconds / 60;
      unsigned long hours = minutes / 60;

      seconds = seconds % 60;
      minutes = minutes % 60;

      String time = String(hours) + ":" + String(minutes) + ":" + String(seconds);

      Serial.println("******************************");
      Serial.println(time);
      Serial.println(" ");

      lcd.setCursor(8, 0);
      lcd.println("Time:" + time);

      get_sensor_data();
    }

    if (kontrol == false)
    {
      startTime = millis();
      currentTime = 0;

      Serial.println("******************************");
      Serial.println("system paused");

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.println("  waiting for   ");
      lcd.setCursor(0, 1);
      lcd.println(" actived system ");
    }
  }
  yield();
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