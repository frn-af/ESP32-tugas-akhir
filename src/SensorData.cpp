/**
 * Created by Farhan Alfathra (frn.af)
 *
 * Github: https://github.com/frn-af/esp32-fermentation
 */
#include "SensorData.h"

static SensorData *instance = NULL;

/*
 * sensor data constructor
 * @param DHTPin pin for DHT sensor
 * @param pHPin pin for PH sensor
 */
SensorData::SensorData(int DHTPin, int pHPin) : DHTPin(DHTPin), pHPin(pHPin)
{
    /*
     * get instance of sensor data
     * @return instance of sensor data
     */
    instance = this;
}

/*
 * init sensor data
 *
 * init dht and ph sensor
 */
void SensorData::init_dht()
{
    dht.setup(DHTPin, DHTesp::DHT22);
    Serial.println("DHT22 Initialized");
}

void SensorData::init_ph()
{
    pinMode(pHPin, INPUT);
    Serial.println("PH Initialized");
}

/*
 * get sensor data
 *
 * get data from dht and ph sensor
 * @return data from dht and ph sensor
 */

TempAndHumidity SensorData::get_dht_data()
{
    TempAndHumidity data = dht.getTempAndHumidity();
    if (isnan(data.temperature) || isnan(data.humidity))
    {
        Serial.println("Failed to read from DHT sensor!");
        data.temperature = 0;
        data.humidity = 0;
    }
    return data;
}

double SensorData::get_ph_data()
{
    nilai_analog_PH = analogRead(pHPin);
    Tegangan_pH = (nilai_analog_PH * 3.3) / 4095;
    pH_Step = (PH4 - PH7) / 3;
    Po = 7 + ((PH7 - Tegangan_pH) / pH_Step);
    return Po;
}
