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

#define READS_PER_SEC 10
#define READS_MILLIS 1000 / READS_PER_SEC

// we expect in 4 seconds from min to max as fast rotation
// (analog range / 4 seconds) / numer of times we read per millisecond
#define FAST_TICK_VALUE ((1024 / 4) / READS_PER_SEC)
#define MEDIUM_TICK_VALUE FAST_TICK_VALUE / 2
#define SLOW_TICK_VALUE MEDIUM_TICK_VALUE / 2

AnalogIn::AnalogIn(uint8_t p_button, bool p_invert, float p_initValue, float p_min, float p_max, float p_minIncrement) :
    NumericInput(),
    m_button(p_button),
    m_invert(p_invert),
    m_value(p_initValue),
    m_min(p_min),
    m_max(p_max),
    m_minIncrement(p_minIncrement),
    m_alpha(0.5f),
    m_rawValue(0.0f),
    m_previousTime(0) {
}

void AnalogIn::init() {
    // analogReadResolution(10);
    m_rawValue = analogRead(A0);
    m_previousTime = millis();
    pinMode(m_button, INPUT_PULLUP);
}

void AnalogIn::handle() {

    if (millis() - m_previousTime >= 100) {
        m_previousTime += READS_MILLIS;

        // Debounce???
        if (digitalRead(m_button) ^ m_invert) {

            float diff = getAnalog() - readAnalog();
            float absoluteDiff = std::fabs(diff);

            // Prevent some jitter
            if (absoluteDiff > 2) {

                float direction = diff >= 0 ? 1.0f : -1.0f;
                float mul = 100.0f;
                if (absoluteDiff < SLOW_TICK_VALUE) {
                    mul = 1.0f;
                } else if (absoluteDiff < MEDIUM_TICK_VALUE) {
                    mul = 10.0f;
                }

                m_value = m_value + direction * m_minIncrement * mul;
                m_value = std::max(m_min, std::min(m_value, m_max));
            }
        } else {
            readAnalog();
        }
    }
}

float AnalogIn::value() const {
    return m_value;
}

float AnalogIn::readAnalog() {
    m_rawValue = m_rawValue + (analogRead(A0) - m_rawValue) * m_alpha;
    return m_rawValue;
}

float AnalogIn::getAnalog() {
    return m_rawValue;
}
