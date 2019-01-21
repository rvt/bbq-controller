#pragma once

#include <stdint.h>
#include <numericinput.h>
#include <analogin.h>
#include <memory>

/**
 * Numeric input using the analog to digital converter
 * The idea is dat we can let thi behave like a rotational encoder with a analog potentiometer
 * The value will only change when the button is pressed.
 * This allows you to ´rewind´ back when rotation is fast and allow for fine grained setting while rotating slow
 * When at the end of the potentiometer just release the button. Re-posotion the potentiometer
 * and press the button for more adjustments
 */
class NumericKnob : public NumericInput {
private:
    std::shared_ptr<AnalogIn>  m_analogIn;
    float m_value;
    float m_min;             // Minimum value possible
    float m_max;             // Maximum value possible
    float m_minIncrement;    // Minimum increment
private:
public:

    NumericKnob(const std::shared_ptr<AnalogIn>& p_analogIn, float m_initValue, float p_min, float p_max, float p_minIncrement);
    void handle();
    void init();

    virtual float value() const;
};