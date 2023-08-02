/**
 * Created by Farhan Alfathra (frn.af)
 *
 * Github: https://github.com/frn-af/esp32-fermentation
 */
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

/*
 * define variable for system
 */
bool kontrol = false;
double temp, hum, ph;
double Setpoint, Input, Output;
double Kp = 2, Ki = 5, Kd = 1;

unsigned long startTime, getDataMillis, currentTime, dataComputeMillis;
unsigned long hours, minutes, seconds;

String title;

/*
 * declare object for system
 */
Network *network;
SensorData *sensorData;
LiquidCrystal_I2C lcd = LiquidCrystal_I2C(0x27, 16, 2);
PID myPID(&Input, &Output, &Setpoint, Kp, Ki, Kd, DIRECT);

/*
 * declare function for system
 */
void init_network();     // init wifi and firebase
void init_sensor_data(); // init dht and ph sensor
void get_sensor_data();  // get data from sensor

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
  /*
   * request data task
   * this task is for request data from firebase
   * request will be called every 20 second with getDataMillis
   * and if firebase ready and wifi connected
   * every request need to be sequential for prevent firebase error
   */
  if (WiFi.status() == WL_CONNECTED && Firebase.ready() && (millis() - getDataMillis > 20000 || getDataMillis == 0))
  {
    // use data millis for tracking when the last request
    // so the system will do multitasking
    getDataMillis = millis();

    kontrol = network->get_kontrol_data(); // get kontrol data from firebase
    Setpoint = network->get_set_point();   // get setpoint data from firebase

    Serial.println("******************************");
    Serial.println("Kontrol: " + String(kontrol));
    Serial.println("Setpoint: " + String(Setpoint));
    Serial.println(" ");

    /*
     * if kontrol is true, system will start to do the fermentation process
     * and if kontrol is false, system will pause the fermentation process
     * and reset the time
     * the controlling process is in the get_sensor_data function
     */
    if (kontrol == true)
    {
      // stopwatch for tracking how long fermentastion process
      currentTime = millis() - startTime;
      // we need to check every loop if the time is change
      // so we can make request every hour and minute change
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

      /*
       * this two condition is need to decrease the firebase request
       * the first condition is for history data that only need to update every hour
       * the second condition is for time data that only need to update every minute
       */
      if (prev_hours != hours)
      {
        title = network->get_history_title();                   // get history title to specific history data
        network->append_suhu_to_history((int)temp, title);      // append temperature data to array
        network->append_kelembaban_to_history((int)hum, title); // append humidity data to array
        network->update_time_history(time, title);              // update time history
      }
      if (prev_minutes != minutes)
      {
        network->update_time(time);
      }

      /*
       * and update temp, humi, ph every loop with datamillis time
       * wicth is 20 second
       */

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

  /*
   * compute data task
   * this task is for compute data from sensor
   * compute will be called every 1 second with dataComputeMillis
   * and if kontrol is true
   */

  if (kontrol == true && (millis() - dataComputeMillis > 1000 || dataComputeMillis == 0))
  {
    dataComputeMillis = millis();

    get_sensor_data();
  }

  yield();
}

/*
 * get sensor data function
 * this function is for get data from sensor
 * and control the fermentation process
 */
void get_sensor_data()
{

  /*
   * get data from sensor
   * and set the data to variable
   */
  TempAndHumidity dhtData = sensorData->get_dht_data();
  temp = dhtData.temperature;
  hum = dhtData.humidity;
  ph = sensorData->get_ph_data();

  /*
   * PID compute
   * this is the main process for controlling the fermentation process
   * the output of this process is the pwm value
   * the input of this process is the temperature
   * the setpoint of this process is the setpoint value
   */
  /*
   * if the temperature is lower than setpoint
   * the system will increase the pwm value
   * and if the temperature is higher than setpoint
   * the system will decrease the pwm value
   * and if the temperature is equal to setpoint
   * the system will keep the pwm value
   */
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

/*
 * init sensor data function
 * this function is for init sensor data
 * and set the pin for sensor
 */
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