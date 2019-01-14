#pragma once

#include <stdint.h>
#include <numericinput.h>

/**
 * Numeric input using the analog to digital converter
 * The idea is dat we can let thi behave like a rotational encoder with a analog potentiometer
 * The value will only change when the button is pressed.
 * This allows you to ´rewind´ back when rotation is fast and allow for fine grained setting while rotating slow
 * When at the end of the potentiometer just release the button. Re-posotion the potentiometer
 * and press the button for more adjustments
 */
class AnalogIn : public NumericInput {
private:
    uint8_t m_button;
    bool m_invert;
    float m_value;
    float m_min;
    float m_max;
    float m_minIncrement;

    float m_alpha;
    float m_previousRaw;
    uint32_t m_previousTime;
private:
    float readAnalogDiff();
public:
    AnalogIn(uint8_t p_button, bool p_invert, float m_initValue, float p_min, float p_max, float p_minIncrement);
    void handle();
    virtual void init();
    virtual float value() const;
};
