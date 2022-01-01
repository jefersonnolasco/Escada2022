
#include "SensorMov.h"
#include "../../enums/sensor.enum.h"

SensorMov::SensorMov(byte s1, byte s2, byte s3) {
    pins[0] = s1;
    pins[1] = s2;
    pins[2] = s3;

    init();
};

void SensorMov::init() {
    for(int x=0; x<3; x++) {
        pinMode(pins[x], INPUT);
        digitalWrite(pins[x], LOW);
        delay(50);
    }
}

eSensor SensorMov::getAtivado() {
    return ativado;
};

void SensorMov::setAtivado(eSensor sensor) {
    ativado = sensor;
};

void SensorMov::pausarSensores() {
    pausarSensor = true;
    lastCheckSensor = millis();
}

bool SensorMov::detectouMovimento() {
    ativado = eSensor::INDEF;

    if (pausarSensor) {
        if (millis()-lastCheckSensor < 2000) {
            return false;
        }
        pausarSensor = false;
    }
  
    for (int x=0; x<3; x++) {
        delay(10);

        if (digitalRead(pins[x])) {
            Serial.println("S"+String(x+1));
            ativado = (eSensor)(int)(x+1);
            break;
        }
    }
  
    return ativado != eSensor::INDEF;
}
