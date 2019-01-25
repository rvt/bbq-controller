#include <cmath>
#include <algorithm>

#include "numericknob.h"

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

#define JITTER_THRESHOLD  0.5


NumericKnob::NumericKnob(const std::shared_ptr<AnalogIn>& p_analogIn,
                         float p_initValue, float p_min, float p_max, float p_velocity) :
    NumericInput(),
    m_analogIn(p_analogIn),
    m_value(p_initValue),
    m_min(p_min),
    m_max(p_max),
    m_velocity(p_velocity) {
}

void NumericKnob::handle() {
    float diff = m_analogIn->valueDiff();

    if (diff < -JITTER_THRESHOLD || diff > JITTER_THRESHOLD) {
        m_value = validValue(m_value + diff * m_velocity);
    }
}

float NumericKnob::validValue(float p_value) const {
    return std::max(m_min, std::min(p_value, m_max));
}

float NumericKnob::value() const {
    return m_value;
}

void NumericKnob::value(float p_value) {
    m_value = validValue(p_value);
}
