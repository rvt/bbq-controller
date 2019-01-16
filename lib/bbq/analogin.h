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
    uint8_t m_button;        // Button pin
    bool m_invert;           // Wether the button should respond inverted
    float m_value;           // Current value after handle
    float m_min;             // Minimum value possible
    float m_max;             // Maximum value possible
    float m_minIncrement;    // Minimum increment

    float m_alpha;           // Alpfa value for filter ( 0 < n <= 1)
    float m_rawValue;        // Previous value
    uint32_t m_previousTime; // Previous time when we handled the analog input
private:
    float readAnalog();
    float getAnalog();
public:
    AnalogIn(uint8_t p_button, bool p_invert, float m_initValue, float p_min, float p_max, float p_minIncrement, float p_alpha = 0.1);
    void handle();
    virtual void init();
    virtual float value() const;
};
