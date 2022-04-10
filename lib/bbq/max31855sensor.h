#pragma once

#include "temperaturesensor.h"
#include <MAX31855.h>
#include <cstdint>

class MAX31855sensor : public TemperatureSensor {
private:
    MAX31855* m_MAX31855;
    float m_lastTemp;
    uint16_t m_faultCode;
    float m_temp_sum;
    uint8_t m_samplecount;
public:
    MAX31855sensor(MAX31855* p_MAX31855);
    virtual float get() const;
    virtual void handle();

    uint16_t faultCode() const;

};
