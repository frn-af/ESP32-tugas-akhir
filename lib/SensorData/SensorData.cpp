#include "SensorData.h"

static SensorData *instance = NULL;

SensorData::SensorData(/* args */)
{
    instance = this;
}

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
