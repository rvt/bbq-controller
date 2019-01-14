#include "analogin.h"

#ifndef UNIT_TEST
#include <Arduino.h>
#else
extern "C" int analogRead(byte);
#endif

AnalogIn::AnalogIn(uint8_t p_button, bool p_invert, float p_initValue, float p_min, float p_max, float p_minIncrement) :
    NumericInput(),
    m_button(p_button),
    m_invert(p_invert),
    m_value(p_initValue),
    m_min(p_min),
    m_max(p_max),
    m_minIncrement(p_minIncrement),
    m_alpha(0.5f),
    m_previousRaw(0.0f),
    m_previousTime(0) {
}

void AnalogIn::init() {
    // analogReadResolution(10);
    m_previousRaw = analogRead(A0);
    m_previousTime = millis();
    pinMode(m_button, INPUT);
}

void AnalogIn::handle() {

    if (millis() - m_previousTime >= 100) {
        m_previousTime += 100;
        readAnalogDiff();

        if (digitalRead(m_button)) {

            float diff = readAnalogDiff();
            float absoluteDiff = fabs(diff);
            float direction = diff >= 0 ? 1.0f : -1.0f;

            if (absoluteDiff < 10) {
                m_value = m_value + direction * m_minIncrement;
            } else if (absoluteDiff < 50) {
                m_value = m_value + direction * m_minIncrement * 10;
            } else {
                m_value = m_value + direction * m_minIncrement * 100;
            }

            m_value = std::max(m_min, std::min(m_value, m_max));
        }
    }
}

float AnalogIn::value() const {
    return m_value;
}

float AnalogIn::readAnalogDiff() {
    float prev = m_previousRaw;
    m_previousRaw = m_previousRaw + (analogRead(A0) - m_previousRaw) * m_alpha;
    return prev - m_previousRaw;
}