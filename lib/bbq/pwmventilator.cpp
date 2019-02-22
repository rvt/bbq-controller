#include "pwmventilator.h"
#include <utils.h>

#ifndef UNIT_TEST
#include <Arduino.h>
#else
extern "C" uint32_t millis();
extern "C" void analogWrite(uint8_t pin, int16_t value);
extern "C" void analogWriteRange(int16_t value);
extern "C" void analogWriteFreq(int16_t value);
extern "C" uint32_t pinMode(uint8_t, uint8_t);
extern "C" void delay(uint16_t);
#define OUTPUT 0
#endif

#define PWM_RANGE  512
#define PWM_FREQUENCY 10000


PWMVentilator::PWMVentilator(uint8_t p_pin, uint8_t p_pwmStart) :
    Ventilator(),
    m_pin(p_pin),
    m_pwmStart(p_pwmStart),
    m_prevPwmValue(0),
    m_on(true) {
    analogWriteRange(PWM_RANGE);
    analogWriteFreq(PWM_FREQUENCY);
    pinMode(p_pin, OUTPUT);
    // Ensure value is in between right values
    setPwmStart(m_pwmStart);
}

void PWMVentilator::setPwmStart(uint8_t p_pwmStart) {
    m_pwmStart = between(p_pwmStart, (uint8_t)0, (uint8_t)100);
}

void PWMVentilator::setOn(bool on) {
    m_on = on;
}



float PWMVentilator::setVentilator() {
    auto  p_speed = speed();

    if (!m_on) {
        p_speed = 0;
    }


    int16_t pwmValue;

    // any speed below 1 is considered off
    if (p_speed < 1) {
        pwmValue = 0;
        p_speed = 0.0f;
    } else {
        // Map 0..100% PWM value to PWM fan can handle
        int16_t startPwm = ((int32_t)PWM_RANGE * (int32_t)m_pwmStart) / 100;
        pwmValue = fmap(p_speed, 0.f, 100.f, startPwm, PWM_RANGE);

        // When the fan was off and turns on give it a little 'kick'
        // Currently a hack, need to find a better way!
        // The delay should only happen when turning on
        if (m_prevPwmValue == 0.0f && p_speed > 0) {
            analogWrite(m_pin, PWM_RANGE);
            delay(100);
        }
    }

    analogWrite(m_pin, pwmValue);
    m_prevPwmValue = p_speed;
    return p_speed;
}
