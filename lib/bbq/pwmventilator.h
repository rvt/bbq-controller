#pragma once
#include <stdint.h>
#include "ventilator.h"

class PWMVentilator : public Ventilator {
private:
    const uint8_t m_pin;
    uint8_t m_pwmStart;
    float m_prevPwmValue;
public:
    /**
     * p_pin : Pin to setup PWM for the fan
     * p_minThreshold : Minimum value where we turn on the fan, below that we set the pwm to 0
     */
    PWMVentilator(uint8_t p_pin, uint8_t p_pwmStart);
    void setPwmStart(uint8_t);
private:
    virtual void setVentilator(const float dutyCycle);
    virtual void handle() {};
};
