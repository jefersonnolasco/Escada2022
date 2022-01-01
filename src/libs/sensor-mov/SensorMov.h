#ifndef SENSOR_H
#define SENSOR_H

#include <Arduino.h>
#include "../../enums/sensor.enum.h"

class SensorMov
{
private:
    byte pins[3];
    int long lastCheckSensor = 0;
    eSensor ativado = eSensor::INDEF;
    bool pausarSensor = false;

    void init();
public:
    SensorMov(byte s1, byte s2, byte s3);

    eSensor getAtivado();
    void setAtivado(eSensor sensor);
    void pausarSensores();
    bool detectouMovimento();
};
#endif