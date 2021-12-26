#include <Arduino.h>
#include <Wire.h>

#include "Sensor.h"

Sensor::Sensor(byte pins[], byte size, void (*msg)(String txt)) {
  this->pins = pins;
  this->size = size;
  this->msg = msg;
}

void Sensor::init() {
  for(int x=0; x<size; x++) {
      pinMode(pins[x], OUTPUT);
      digitalWrite(pins[x], LOW);
  };
  
  delay(500);
  Wire.begin();

  for(int x=0; x<size; x++) {
    pinMode(pins[x], INPUT);
    delay(150);
    sensores[x].init(true);
    delay(100);
    sensores[x].setAddress((uint8_t)x+1);
    sensores[x].setTimeout(500);

    sensores[x].setSignalRateLimit(0.1);  
    sensores[x].setVcselPulsePeriod(VL53L0X::VcselPeriodPreRange, 18);
    sensores[x].setVcselPulsePeriod(VL53L0X::VcselPeriodFinalRange, 14);
  }

  byte count = 0;
  for (byte i = 1; i < 30; i++)
  {
    Wire.beginTransmission (i);
    if (Wire.endTransmission() == 0)
    {
      count++;
      delay(1);
    }
  }

  msg("Found "+String(count)+" device(s).");
};

int Sensor::checkSensor() {
  for(int x=0; x<size; x++) {
    if (checkUniqueSensor(x)) {
      return x;
    }
  }
  return -1;
}

boolean Sensor::checkUniqueSensor(int idx) {
  int qtd = 0;
  for(int x=0; x<5; x++) {
    long distance = sensores[idx].readRangeSingleMillimeters();
    if (sensores[x].timeoutOccurred())
    {
        continue;
    }

    if (distance>distMin && distance<distMax) {
        qtd++;
    }
  }
  return qtd >= 4;
};