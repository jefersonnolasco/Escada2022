#ifndef SENSOR_H
#define SENSOR_H
#include <VL53L0X.h>

class Sensor
{
private:
  byte *pins;
  byte size;
  void (*msg)(String txt);
  VL53L0X sensores[3];

  long distMin = 100;
  long distMax = 2200; 
  
  boolean checkUniqueSensor(int idx);
public:
  Sensor(byte pins[], byte size, void (*msg)(String txt));
  void init();
  int checkSensor();
};
#endif