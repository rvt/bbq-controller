#include "analogin.h"

#include <cmath>
#include <algorithm>

#ifndef UNIT_TEST
#include <Arduino.h>
#else
extern "C" uint32_t millis();
extern "C" uint32_t analogRead(uint8_t);
extern "C" uint32_t pinMode(uint8_t, uint8_t);
extern "C" uint32_t digitalRead(uint8_t);
#define INPUT_PULLUP 2
#define A0 0xa0
#endif

AnalogIn::AnalogIn(float p_alpha) :
    m_alpha(p_alpha),
    m_rawValue(0.0f),
    m_diff(0.0f) {
}

void AnalogIn::init() {
    // analogReadResolution(10);
    m_rawValue = analogRead(A0);
}

void AnalogIn::handle() {
    auto current = m_rawValue;
    m_rawValue = m_rawValue + (analogRead(A0) - m_rawValue) * m_alpha;
    m_diff = m_rawValue - current;
}

float AnalogIn::value() const {
    return m_rawValue;
}

float AnalogIn::valueDiff() const {
    return m_diff;
}
