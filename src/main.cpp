#include <Arduino.h>
#include "Network.h"
#include "SensorData.h"
#include <Ticker.h>
#include <LiquidCrystal_I2C.h>
#include <PID_v1.h>
/*
 * define pin for hardware
 * pin for DHT, PH declare here so we dont need to change the lib
 */
const int PWM = 18;
const int DHTPin = 19;
const int pHPin = 32;

bool kontrol = false;

double temp, hum, ph;
double Setpoint, Input, Output;
double Kp = 2, Ki = 5, Kd = 1;

unsigned long startTime, getDataMillis, currentTime, dataComputeMillis;
unsigned long hours, minutes, seconds;

String title;

Network *network;
SensorData *sensorData;
LiquidCrystal_I2C lcd = LiquidCrystal_I2C(0x27, 16, 2);
PID myPID(&Input, &Output, &Setpoint, Kp, Ki, Kd, DIRECT);

void init_network();
void init_sensor_data();
void get_sensor_data();

void setup()
{
  Serial.begin(115200);
  Serial.println("System starting...");
  lcd.init();
  lcd.backlight();
  pinMode(PWM, OUTPUT);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.println("System starting.");

  init_network();

  lcd.setCursor(0, 0);
  lcd.println("Network ready...");
  lcd.setCursor(0, 1);
  lcd.println("init system.....");

  myPID.SetMode(AUTOMATIC);

  init_sensor_data();

  startTime = millis();
}

void loop()
{

  if (WiFi.status() == WL_CONNECTED && Firebase.ready() && (millis() - getDataMillis > 20000 || getDataMillis == 0))
  {
    getDataMillis = millis();

    kontrol = network->get_kontrol_data();
    Setpoint = network->get_set_point();

    Serial.println("******************************");
    Serial.println("Kontrol: " + String(kontrol));
    Serial.println("Setpoint: " + String(Setpoint));
    Serial.println(" ");

    if (kontrol == true)
    {
      currentTime = millis() - startTime;

      unsigned long prev_hours = hours;
      unsigned long prev_minutes = minutes;
      seconds = currentTime / 1000;
      minutes = seconds / 60;
      hours = minutes / 60;

      seconds = seconds % 60;
      minutes = minutes % 60;

      String time = String(hours) + " jam " + String(minutes) + " menit";

      Serial.println("******************************");
      Serial.println(time);
      Serial.println(" ");

      lcd.setCursor(8, 0);
      lcd.println("Time:" + time);

      if (prev_hours != hours)
      {
        title = network->get_history_title();
        network->append_suhu_to_history((int)temp, title);
        network->append_kelembaban_to_history((int)hum, title);
        network->update_time_history(time, title);
      }

      if (prev_minutes != minutes)
      {
        network->update_time(time);
      }

      network->update_data((int)temp, (int)hum, (int)ph);
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

  if (kontrol == true && (millis() - dataComputeMillis > 1000 || dataComputeMillis == 0))
  {
    dataComputeMillis = millis();

    get_sensor_data();
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