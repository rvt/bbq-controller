#pragma once

#include "temperaturesensor.h"
#include <Adafruit_MAX31855.h>

class MAX31855sensor : public TemperatureSensor {
private:
    Adafruit_MAX31855* m_MAX31855;
    float m_lastTemp;
public:
    MAX31855sensor(Adafruit_MAX31855* p_adafruit_MAX31855);
    virtual float get() const;
    virtual void handle();

};
