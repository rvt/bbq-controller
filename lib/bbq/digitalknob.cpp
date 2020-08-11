

#include "digitalknob.h"
#include <stdint.h>
#include <algorithm>

#ifndef UNIT_TEST
#include <Arduino.h>
#else
extern "C" uint32_t millis();
extern "C" uint32_t pinMode(uint8_t, uint8_t);
extern "C" uint32_t digitalRead(uint8_t);
#define INPUT_PULLUP 2
#define A0 0xa0
#endif

// Bit positions (std::bitset) for each type of press
#define DIGITAL_KNOB_CURRENT 0
#define DIGITAL_KNOB_IS_SINGLE_CLICK 1
#define DIGITAL_KNOB_IS_DOUBLE_CLICK 2
#define DIGITAL_KNOB_IS_LONG_PRESS 3
#define DIGITAL_KNOB_IS_EDGE_UP 4
#define DIGITAL_KNOB_IS_EDGE_DOWN 5

//Hysteresis to detect low/high states for debounce
constexpr int16_t DIGITAL_KNOB_HIGH = 200;
constexpr int16_t DIGITAL_KNOB_LOW = 255-DIGITAL_KNOB_HIGH;

// Bitmasks for detething the type of press based on the type of press 32 or 64 bit
#define DIGITAL_KNOB_SINGLE_CLICK_AMASK 0b00000111111111111000000000000000
#define DIGITAL_KNOB_SINGLE_CLICK_TMASK 0b00000000111111100000000000000000

#define DIGITAL_KNOB_DOUBLE_CLICK_AMASK 0b00001111111100000000000111111110
#define DIGITAL_KNOB_DOUBLE_CLICK_TMASK 0b00000011110000000000000001111000
#define DIGITAL_KNOB_LONG_PRESS_AMASK   0b11111111111111111111111111111111
#define DIGITAL_KNOB_LONG_PRESS_TMASK   0b00000001111111111111111111111111

DigitalKnob::DigitalKnob(uint8_t p_pin) : DigitalKnob(p_pin, true, 150) {
}

DigitalKnob::DigitalKnob(uint8_t p_pin, bool p_invert, int16_t p_alpha) :
    DigitalInput(),
    m_pin(p_pin),
    m_invert(p_invert),
    m_alpha(p_alpha),
    m_rawValue(0x00),
    m_value({}),
m_status({0x00}) {
}

void DigitalKnob::init() {
    pinMode(m_pin, INPUT_PULLUP);
}

void DigitalKnob::handle() {
    // Debounce input
    int16_t correction = ((digitalRead(m_pin)^m_invert ? 0xff : 0) - m_rawValue) * 100;
    m_rawValue = m_rawValue + correction / m_alpha;
    m_rawValue = std::max((int16_t)0, std::min(m_rawValue, (int16_t)0xff));

    bool previousButtonState = m_value[DIGITAL_KNOB_CURRENT];

    // Hysteresis for digital input
    if (m_rawValue > DIGITAL_KNOB_HIGH) {
        m_value[DIGITAL_KNOB_CURRENT] = true;
    } else if (m_rawValue < DIGITAL_KNOB_LOW) {
        m_value[DIGITAL_KNOB_CURRENT] = false;
    } else {
        return;
    }


    // Shift and insert current bit into register
    m_status.m_status32 = m_status.m_status32 << 1;
    m_status.m_status8[0] = m_status.m_status8[0] | m_value[DIGITAL_KNOB_CURRENT];

    // Reset buttons if they where not captured for a duration
    if (m_status.m_status32 == 0x00) {
       // resetButtons();
    }

    // Detect up/down edges
    m_value[DIGITAL_KNOB_IS_EDGE_UP] =   ( m_value[DIGITAL_KNOB_CURRENT] && !previousButtonState) || m_value[DIGITAL_KNOB_IS_EDGE_UP];
    m_value[DIGITAL_KNOB_IS_EDGE_DOWN] = (!m_value[DIGITAL_KNOB_CURRENT] &&  previousButtonState) || m_value[DIGITAL_KNOB_IS_EDGE_DOWN];
    
    // detect long press
    if ((m_status.m_status32 | DIGITAL_KNOB_LONG_PRESS_AMASK) == DIGITAL_KNOB_LONG_PRESS_AMASK &&
        (m_status.m_status32 & DIGITAL_KNOB_LONG_PRESS_TMASK) == DIGITAL_KNOB_LONG_PRESS_TMASK) {
        m_value[DIGITAL_KNOB_IS_LONG_PRESS] = true;
    }

    // Detect single click
    if ((m_status.m_status32 | DIGITAL_KNOB_SINGLE_CLICK_AMASK) == DIGITAL_KNOB_SINGLE_CLICK_AMASK &&
        (m_status.m_status32 & DIGITAL_KNOB_SINGLE_CLICK_TMASK) == DIGITAL_KNOB_SINGLE_CLICK_TMASK) {
        m_value[DIGITAL_KNOB_IS_SINGLE_CLICK] = true;
        m_status.m_status32 = 0x00;
    }

    // Detect double click
    if ((m_status.m_status32 | DIGITAL_KNOB_DOUBLE_CLICK_AMASK) == DIGITAL_KNOB_DOUBLE_CLICK_AMASK &&
        (m_status.m_status32 & DIGITAL_KNOB_DOUBLE_CLICK_TMASK) == DIGITAL_KNOB_DOUBLE_CLICK_TMASK) {
        m_value[DIGITAL_KNOB_IS_DOUBLE_CLICK] = true;
        m_status.m_status32 = 0x00;
    }
}

bool DigitalKnob::current() const {
    return m_value[DIGITAL_KNOB_CURRENT];
}

bool DigitalKnob::isSingle() const {
    bool v = m_value[DIGITAL_KNOB_IS_SINGLE_CLICK];
    m_value[DIGITAL_KNOB_IS_SINGLE_CLICK] = false;
    return v;
}

bool DigitalKnob::isEdgeUp() const {
    bool v = m_value[DIGITAL_KNOB_IS_EDGE_UP];
    m_value[DIGITAL_KNOB_IS_EDGE_UP] = false;
    return v;
}
bool DigitalKnob::isEdgeDown() const {
    bool v = m_value[DIGITAL_KNOB_IS_EDGE_DOWN];
    m_value[DIGITAL_KNOB_IS_EDGE_DOWN] = false;
    return v;
}
bool DigitalKnob::isEdge() const {
    bool v1 = m_value[DIGITAL_KNOB_IS_EDGE_DOWN];
    bool v2 = m_value[DIGITAL_KNOB_IS_EDGE_UP];
    m_value[DIGITAL_KNOB_IS_EDGE_DOWN] = false;
    m_value[DIGITAL_KNOB_IS_EDGE_UP] = false;
    return v1 || v2;
}
bool DigitalKnob::isDouble() const {
    bool v = m_value[DIGITAL_KNOB_IS_DOUBLE_CLICK];
    m_value[DIGITAL_KNOB_IS_DOUBLE_CLICK] = false;
    return v;
}

bool DigitalKnob::isLong() const {
    bool v = m_value[DIGITAL_KNOB_IS_LONG_PRESS];
    m_value[DIGITAL_KNOB_IS_LONG_PRESS] = false;
    return v;
}

void DigitalKnob::resetButtons() const {
    m_value = 0x00 | m_value[DIGITAL_KNOB_CURRENT];
}

void DigitalKnob::reset() const {
    m_status.m_status32 = 0x00;
}
