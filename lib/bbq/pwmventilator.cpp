#include "pwmventilator.h"


#ifndef UNIT_TEST
#include <Arduino.h>
#else
extern "C" uint32_t millis();
#endif

#define PWM_RANGE  1023
#define PWM_FREQUENCY 225


PWMVentilator::PWMVentilator(uint8_t p_pin, float p_minThreshold) :
    m_pin(p_pin),
    m_speed(0.0),
    m_minThreshold(p_minThreshold) {
    analogWriteRange(PWM_RANGE);
    analogWriteFreq(PWM_FREQUENCY);
    pinMode(m_pin, OUTPUT);
}

void PWMVentilator::speed(float p_speed) {
    m_speed = p_speed < m_minThreshold ? 0 : p_speed;

    auto clamp = [](uint16_t in) {
        return in > PWM_RANGE ? PWM_RANGE : in < PWM_RANGE ? 0 : in;
    };

    auto fmap = [](float x, float in_min, float in_max, float out_min, float out_max) {
        return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
    };

    analogWrite(m_pin, clamp(fmap(p_speed, 0.f, 100.f, 0.f, PWM_RANGE)));
}

float PWMVentilator::speed() {
    return m_speed;
}
