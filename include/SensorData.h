#ifndef SensorData_H_
#define SensorData_H_

#include <DHTesp.h>

class SensorData
{
private:
    /* data */
    DHTesp dht;
    int DHTPin;
    int pHPin;

    float Po = 0;
    float pH_Step;
    int nilai_analog_PH;
    double Tegangan_pH;

    float PH4 = 3.24;
    float PH7 = 2.7;

public:
    SensorData(int DHTPin, int pHPin);
    void init_dht();
    void init_ph();
    TempAndHumidity get_dht_data();
    double get_ph_data();
};

#endif // !SensorData_H_