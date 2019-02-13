#include "pwmventilator.h"

#include <utils.h>
#ifndef UNIT_TEST
#include <Arduino.h>
#else
extern "C" uint32_t millis();
#endif

#define PWM_RANGE  512
#define PWM_FREQUENCY 10000


PWMVentilator::PWMVentilator(uint8_t p_pin, float p_minThreshold) :
    Ventilator(),
    m_pin(p_pin),
    m_minThreshold(p_minThreshold) {
    analogWriteRange(PWM_RANGE);
    analogWriteFreq(PWM_FREQUENCY);
    pinMode(p_pin, OUTPUT);
}

float PWMVentilator::setVentilator() {
    auto p_speed = speed();
    auto neededRpm = p_speed < m_minThreshold ? 0 : p_speed;
    // TODO under 40% we need to switch to on/off method
    analogWrite(m_pin, between((int16_t)fmap(neededRpm, 40.f, 100.f, 0.f, PWM_RANGE), (int16_t)0, (int16_t)PWM_RANGE));
    return neededRpm;
}
