#pragma once
#include <stdint.h>
#include "ventilator.h"

class PWMVentilator : public Ventilator {
private:
    uint8_t m_pin;
    float m_minThreshold;
public:
    /**
     * p_pin : Pin to setup PWM for the fan
     * p_minThreshold : Minimum value where we turn on the fan, below that we set the pwm to 0
     */
    PWMVentilator(uint8_t p_pin, float p_minThreshold);
private:
    virtual float setVentilator();
};
