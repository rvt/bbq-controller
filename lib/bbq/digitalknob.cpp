

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
#define DIGITAL_KNOB_IS_CLICK 4

//Hysteresis to detect low/high states for debounce
#define DIGITAL_KNOB_HIGH 200
#define DIGITAL_KNOB_LOW 100

// Bitmasks for detething the type of press based on the type of press 32 or 64 bit
#define DIGITAL_KNOB_SINGLE_CLICK_MASK  0b0000011111111000000000000000000
#define DIGITAL_KNOB_SINGLE_CLICK_TMASK 0b0000000111100000000000000000000

#define DIGITAL_KNOB_DOUBLE_CLICK_MASK  0b0111111110000000000000011111111
#define DIGITAL_KNOB_DOUBLE_CLICK_TMASK 0b0001111000000000000000000111100
#define DIGITAL_KNOB_LONG_PRESS_MASK    0b000000000000011111111111111111111111111111111111111111111111111

DigitalKnob::DigitalKnob(uint8_t p_pin) : DigitalKnob(p_pin, 150) {
}

DigitalKnob::DigitalKnob(uint8_t p_pin, int16_t p_alpha) :
    DigitalInput(),
    m_rawValue(0x00),
    m_value({}),
    m_status({0x00}),
    m_pin(p_pin),
    m_alpha(p_alpha) {
}

void DigitalKnob::init() {
    pinMode(m_pin, INPUT_PULLUP);
}

void DigitalKnob::handle() {
    // Debounce input
    auto correction = ((digitalRead(m_pin) ? 0xff : 0) - m_rawValue) * 100;
    m_rawValue = m_rawValue + correction / m_alpha;
    m_rawValue = std::max((int16_t)0, std::min(m_rawValue, (int16_t)0xff));

    // Check status current button
    bool current = m_value[DIGITAL_KNOB_CURRENT];

    // Hysteresis for digital input
    if (m_rawValue > DIGITAL_KNOB_HIGH) {
        m_value.set(DIGITAL_KNOB_CURRENT);
    } else if (m_rawValue < DIGITAL_KNOB_LOW) {
        m_value.reset(DIGITAL_KNOB_CURRENT);
    }

    // Shift and insert current bit into register
    m_status.m_status64 = m_status.m_status64 << 1;
    m_status.m_status8[0] = m_status.m_status8[0] | m_value[DIGITAL_KNOB_CURRENT];

    // detect long press
    if (m_status.m_status64 == DIGITAL_KNOB_LONG_PRESS_MASK) {
        m_value.set(DIGITAL_KNOB_IS_LONG_PRESS);
        m_status.m_status64 = 0x00;
    }

    // Detect single click
    if ((m_status.m_status32[0] | DIGITAL_KNOB_SINGLE_CLICK_MASK) == DIGITAL_KNOB_SINGLE_CLICK_MASK &&
        (m_status.m_status32[0] &  DIGITAL_KNOB_SINGLE_CLICK_TMASK) == DIGITAL_KNOB_SINGLE_CLICK_TMASK) {
        m_value.set(DIGITAL_KNOB_IS_SINGLE_CLICK);
        m_status.m_status64 = 0x00;
    }

    // Detect double click
    if ((m_status.m_status32[0] | DIGITAL_KNOB_DOUBLE_CLICK_MASK) == DIGITAL_KNOB_DOUBLE_CLICK_MASK &&
        (m_status.m_status32[0] &  DIGITAL_KNOB_DOUBLE_CLICK_TMASK) == DIGITAL_KNOB_DOUBLE_CLICK_TMASK) {
        m_value.set(DIGITAL_KNOB_IS_DOUBLE_CLICK);
        m_status.m_status64 = 0x00;
    }
}

bool DigitalKnob::current() const {
    return m_value[DIGITAL_KNOB_CURRENT];
}

bool DigitalKnob::isSingle() const {
    return m_value[DIGITAL_KNOB_IS_SINGLE_CLICK];
}

bool DigitalKnob::isDouble() const {
    return m_value[DIGITAL_KNOB_IS_DOUBLE_CLICK];
}

bool DigitalKnob::isLong() const {
    return m_value[DIGITAL_KNOB_IS_LONG_PRESS];
}

void DigitalKnob::resetButtons() {
    m_value = 0x00;
}
