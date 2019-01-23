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

#define READS_PER_SEC 10
#define READS_MILLIS (1000 / READS_PER_SEC)

// we expect in 4 seconds from min to max as fast rotation
// (analog range / 4 seconds) / numer of times we read per millisecond
#define FAST_TICK_VALUE ((1024 / 4) / READS_PER_SEC)
#define MEDIUM_TICK_VALUE FAST_TICK_VALUE / 2
#define SLOW_TICK_VALUE MEDIUM_TICK_VALUE / 2




NumericKnob::NumericKnob(const std::shared_ptr<AnalogIn>& p_analogIn,
                         float p_initValue, float p_min, float p_max, float p_minIncrement) :
    NumericInput(),
    m_analogIn(p_analogIn),
    m_value(p_initValue),
    m_min(p_min),
    m_max(p_max),
    m_minIncrement(p_minIncrement) {
}

void NumericKnob::handle() {
    float diff = m_analogIn->valueDiff();
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
}

float NumericKnob::value() const {
    return m_value;
}
