#pragma once
#include <stdint.h>
#include "ventilator.h"

class PWMVentilator : public Ventilator {
private:
    const uint8_t m_pin;
    uint16_t m_pwmStart;
    uint16_t m_pwmMinimum;
    float m_prevPwmValue;
    uint8_t m_pwmChannel;
public:
    /**
     * p_pin : Pin to setup PWM for the fan
     * p_pwmStart : Minimum value where the fab can run, below that we set the pwm to 0 (0..100%)
     */
    PWMVentilator(uint8_t p_pin, uint8_t p_pwmStart);
    PWMVentilator(uint8_t p_pin, uint8_t p_pwmStart, uint8_t p_pwmChannel);
    PWMVentilator(uint8_t p_pin, uint8_t p_pwmStart, uint8_t m_pwmMinimum, uint8_t p_pwmChannel);

    virtual ~PWMVentilator();

private:
    virtual void setVentilator(float dutyCycle);
    virtual void handle(const uint32_t millis) {};
};
