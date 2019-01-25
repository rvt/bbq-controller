#pragma once

#include <stdint.h>
#include <numericinput.h>
#include <analogin.h>
#include <memory>

/**
 * Numeric input using the analog to digital converter
 * The idea is dat we can let this behave like a rotational encoder with a analog potentiometer
 * The value will only change when the button is pressed (by calling handle() or not )
 * This allows you to ´rewind´ back when rotation is fast and allow for fine grained setting while rotating slow
 * When at the end of the potentiometer just release the button. Re-posotion the potentiometer
 * and press the button for more adjustments
 */
class NumericKnob : public NumericInput {
private:
    const std::shared_ptr<AnalogIn>  m_analogIn;
    float m_value;
    const float m_min;             // Minimum value possible
    const float m_max;             // Maximum value possible
    const float m_velocity;    // Minimum increment
private:
    float validValue(float p_value) const ;
public:

    NumericKnob(
        const std::shared_ptr<AnalogIn>& p_analogIn,
        float m_initValue,
        float p_min,
        float p_max,
        float p_velocity);
    /*
    * Call at a rate of 50 handles7sec. This is needed because the difference of values is used and
    * they are based on timings
    */
    void handle();

    virtual float value() const;
    virtual void value(float p_value);
};
