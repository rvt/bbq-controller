#pragma once
#include <stdint.h>
#include "ventilator.h"

// This class needs soem serioualy cleaning up because there is to much code in here that depends
// on different hardware platforms. For now, let´s expriment and make this work
class PWMVentilator : public Ventilator {
private:
    const uint8_t m_pin;
    float m_prevPwmValue;
    uint32_t m_kickTime;
    bool m_isKick;
    const uint32_t m_maxDuty;
    const uint8_t m_dutyStart;
    const uint8_t m_pwmChannel;
public:
    /**
     * p_pin : Pin to setup PWM for the fan
     * p_dutyStart : Minimum duty cycle where the fan can run, below that we set the pwm to 0 (0..100%)
     */
    PWMVentilator(uint8_t p_pin, uint8_t p_dutyStart);
    PWMVentilator(uint8_t p_pin, uint8_t p_dutyStart, uint8_t p_pwmChannel);
    PWMVentilator(uint8_t p_pin, uint8_t p_dutyStart, uint8_t m_dutyMinimum, uint8_t p_pwmChannel);

    virtual ~PWMVentilator();

private:
    virtual void setVentilator(float dutyCycle);
    virtual void handle(const uint32_t millis) {};
};
