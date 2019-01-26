#pragma once

#include "temperaturesensor.h"
#include <Adafruit_MAX31865.h>

class MAX31865sensor : public TemperatureSensor {
private:
    Adafruit_MAX31865* m_MAX31865;
    const float m_RNominal;
    const float m_Rref;
    float m_lastTemp;
public:
    MAX31865sensor(Adafruit_MAX31865* p_adafruit_MAX31865, float p_RNominal, float p_Rref);
    virtual float get() const;
    virtual void handle();

};
