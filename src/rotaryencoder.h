#pragma once

#include <digitalknob.h>
#include <NumericInput.h>
#include <Arduino.h>

class RotaryEncoder  : public NumericInput {
private:
    DigitalKnob* m_pin1;
    DigitalKnob* m_pin2;
    int16_t m_pos;
    int16_t m_posLast;
public:
    RotaryEncoder() : RotaryEncoder(nullptr, nullptr) {

    }
    RotaryEncoder(DigitalKnob* p_pin1, DigitalKnob* p_pin2) : NumericInput(),
        m_pin1(p_pin1),
        m_pin2(p_pin2),
        m_pos(0),
        m_posLast(0) {

    }

    virtual void handle() {
        if (m_pin1 == nullptr || m_pin2 == nullptr) {
            return;
        }

        m_pin1->handle();
        m_pin2->handle();
        // Quadrature decoder using xor
        // https://howtomechatronics.com/tutorials/arduino/rotary-encoder-works-use-arduino/
        // 0 ^ 0 = 1
        // 0 ^ 1 = 0
        // 1 ^ 0 = 0
        // 1 ^ 1 = 1
        int8_t direction = 0;

        if (m_pin1->isEdge()) {
            direction = 1;
            m_pin2->reset();
        } else if (m_pin2->isEdge()) {
            direction = -1;
        }

        if (direction) {
            if (m_pin1->current() ^ m_pin2->current()) {
                m_pos = m_pos + direction;
            } else {
                m_pos = m_pos - direction;
            }
        }
    }

    int16_t difference() {
        int16_t difference = m_pos - m_posLast;
        m_posLast = m_pos;
        return difference;
    }

    virtual float value() const {
        return m_pos;
    };

    virtual void value(float value) {
        m_posLast = value;
        m_pos = value;
    };

};
