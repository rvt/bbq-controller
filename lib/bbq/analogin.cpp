#include "analogin.h"

#ifndef UNIT_TEST
#include <Arduino.h>
#else
extern "C" int analogRead(byte);
#endif

AnalogIn::AnalogIn(float p_min, float p_max) :
    m_min(p_min),
    m_max(p_max),
    m_alpha(0.1f),
    m_value(p_min) {
}

void AnalogIn::handle() {
    auto fmap = [](float x, float in_min, float in_max, float out_min, float out_max) {
        return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
    };
    float fVal = fmap(analogRead(A0), 0, 1023, m_min, m_max);
    m_value = m_value + (fVal - m_value) * m_alpha;
}

float AnalogIn::value() const {
    return m_value;
}