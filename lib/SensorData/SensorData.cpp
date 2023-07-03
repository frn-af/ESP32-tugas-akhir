#include "SensorData.h"

static SensorData *instance = NULL;

SensorData::SensorData(/* args */)
{
    instance = this;
}

void SensorData::initDHT()
{
    dht.setup(DHTPin, DHTesp::DHT22);
    Serial.println("DHT22 Initialized");
}

TempAndHumidity SensorData::getDHTData()
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
