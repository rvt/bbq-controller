#pragma once
#include <stdint.h>
#include "ventilator.h"

class OnOffVentilator : public Ventilator {
private:
    const uint8_t m_pin;
    uint32_t m_period;
public:
    /**
     * p_pin : Pin to setup PWM for the fan
     * p_period : Minimum value where we turn on the fan, below that we set the pwm to 0
     */
    OnOffVentilator(uint8_t p_pin, uint32_t p_period);
    virtual void handle();
private:
    virtual void setVentilator(const float dutyCycle);
};
