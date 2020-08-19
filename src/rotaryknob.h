#pragma once

#include <digitalknob.h>
#include <rotaryencoder.h>
#include <numericinput.h>


class RotaryKnob : public NumericInput {
    DigitalKnob* m_pin1;
    DigitalKnob* m_pin2;
    float m_value;
    float m_minValue;
    float m_maxValue;
    float m_velocity;
    RotaryEncoder m_encoder;
public:
    RotaryKnob(
        DigitalKnob* p_pin1,
        DigitalKnob* p_pin2,
        float p_value,
        float p_minValue,
        float p_maxValue,
        float p_velocity);

    // Call this often enough so that the quadratuur encoder will work
    virtual void handle();
    virtual float value() const;
    virtual void value(float p_value);
};
