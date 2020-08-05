#pragma once

#include <rotaryknob.h>
#include <utils.h>


        DigitalKnob *m_pin1;
        DigitalKnob *m_pin2;
        float m_value;
        float m_minValue;
        float m_maxValue;
        float m_velocity;
        RotaryEncoder m_encoder;

RotaryKnob::RotaryKnob(
        DigitalKnob *p_pin1, 
        DigitalKnob *p_pin2, 
        float p_value,
        float p_minValue, 
        float p_maxValue,
        float p_velocity ) : 
        m_pin1(p_pin1),
        m_pin2(p_pin2),
        m_value(p_value),
        m_minValue(p_minValue),
        m_maxValue(p_maxValue),
        m_velocity(p_velocity)
         {
                m_encoder = RotaryEncoder(p_pin1, p_pin2);
        }

 void RotaryKnob::handle() {
        m_encoder.handle();
        float difference = m_encoder.difference() * m_velocity;
        m_value = between(m_value+difference, m_minValue, m_maxValue);
}

 float RotaryKnob::value() const {
        return m_value;
}

 void RotaryKnob::value(float newvalue) {
        m_value = newvalue;
}
