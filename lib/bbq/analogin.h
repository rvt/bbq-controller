#pragma once

#include <stdint.h>

/**
 * Numeric input using the analog to digital converter
 * The idea is dat we can let thi behave like a rotational encoder with a analog potentiometer
 * The value will only change when the button is pressed.
 * This allows you to ´rewind´ back when rotation is fast and allow for fine grained setting while rotating slow
 * When at the end of the potentiometer just release the button. Re-posotion the potentiometer
 * and press the button for more adjustments
 */
class AnalogIn {
private:
    const float m_alpha;           // Alpfa value for filter ( 0 < n <= 1)
    float m_rawValue;        // Previous value
    float m_diff;
public:
    AnalogIn(float p_alpha = 0.1);
    void handle();
    void init();
    float value() const;
    float valueDiff() const;
};
