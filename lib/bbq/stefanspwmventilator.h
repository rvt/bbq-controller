#pragma once

#if defined(ESP8266)

#include <stdint.h>
#include "ventilator.h"

class StefansPWMVentilator : public Ventilator {
private:
    const uint8_t m_pin;
    uint8_t m_pwmStart;
    float m_pwmValue;
    uint32_t io_info[5][3];
    float m_currentPwmValue;
    uint32_t m_lastMillis;
public:
    /**
     * p_pin : Pin to setup PWM for the fan
     * p_pwmStart : Minimum value where we turn on the fan, below that we set the pwm to 0
     */
    StefansPWMVentilator(uint8_t p_pin, uint8_t p_pwmStart);
    void setPwmStart(uint8_t);
private:
    virtual void setVentilator(const float dutyCycle);
    virtual void handle(const uint32_t millis);
};
#endif