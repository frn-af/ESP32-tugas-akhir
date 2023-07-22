#ifndef SensorData_H_
#define SensorData_H_

#include <DHTesp.h>

class SensorData
{
private:
    /* data */
    DHTesp dht;
    const int DHTPin = 19;
    const int pHPin = 32;
    float Po = 0;
    float pH_Step;
    int nilai_analog_PH;
    double Tegangan_pH;

    float PH4 = 3.24;
    float PH7 = 2.7;

public:
    SensorData(/* args */);
    void initDHT();
    void initPH();
    TempAndHumidity getDHTData();
    double getPHData();
};

#endif // !SensorData_H_