#ifndef SensorData_H_
#define SensorData_H_

#include <DHTesp.h>

class SensorData
{
private:
    /* data */
    DHTesp dht;
    const int DHTPin = 21;

public:
    SensorData(/* args */);
    void initDHT();
    TempAndHumidity getDHTData();
};

#endif // !SensorData_H_