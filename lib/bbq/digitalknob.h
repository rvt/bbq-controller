#pragma once

#include <stdint.h>
#include <digitalinput.h>
#include <memory>
#include <bitset>


union BITS64 {
    uint64_t m_status64;
    uint32_t m_status32[2];
    uint8_t m_status8[4];
};

/**
 * DigitalKnob handles input of a simple button connected to a digital pin.
 * It will beable to detect single click, double click or long press
 * It is protected against debounce
 */
class DigitalKnob : public DigitalInput {
private:
    int16_t m_rawValue;
    mutable std::bitset<4> m_value;
    BITS64 m_status;
    uint8_t m_pin;
    int16_t  m_alpha;
    bool m_invert;
public:

    DigitalKnob(uint8_t p_pin);
    DigitalKnob(uint8_t p_pin, bool p_invert, int16_t p_alpha);
    void handle();
    void init();

    virtual bool current() const;
    virtual bool isSingle() const;
    virtual bool isDouble() const;
    virtual bool isLong() const;
    void resetButtons() const;
    std::bitset<64> intern() {
        std::bitset<64> v(m_status.m_status64);
        return v;
    }
    std::bitset<4> presses() {
        std::bitset<4> v(m_value);
        return v;
    }
};
