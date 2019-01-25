#pragma once

#include <stdint.h>
#include <digitalinput.h>
#include <memory>
#include <bitset>




/**
 * DigitalKnob handles input of a simple button connected to a digital pin.
 * It will beable to detect single click, double click or long press edgeUp and edgeDown situations
 * It is protected against debounce
 *
 * Ensure you call the handle function 50 times/sec to handle the single/double click timing correctly
 *
 * button states are captured and remembered for as long we have bit´s in the train of up to 50 ticks
 * After that all states will be reset to 0.
 */
class DigitalKnob : public DigitalInput {
private:
    union BITS32 {
        uint32_t m_status32;
        uint8_t m_status8[4];
    };
    const uint8_t m_pin;
    const bool m_invert;
    const int16_t m_alpha;
    int16_t m_rawValue;
    mutable std::bitset<6> m_value;
    mutable BITS32 m_status;
public:
    /**
     * Build a button with standard coviguration
     * Input it standard inverted for normal pull-up configuration
     * param: Pin number
     */
    DigitalKnob(uint8_t p_pin);
    /**
     * Build a button with specific conviguration
     * param: Pin number
     * param: Invert the pin input, when pull-ip stndard config is inverted
     * param: Alpha wilter value default 150, the hihger the value the more filtering on the digital input
     */
    DigitalKnob(uint8_t p_pin, bool p_invert, int16_t p_alpha);

    /**
     * Call the handle function 50 times/sec for correct button detection
     */
    void handle();

    /**
     * Initialise the button and enable the pin modus
     * It sets the pin mode to INPUT_PULLUP but for the esp8266 that didn´t work,
     * so still use a external pullup.
     */
    void init();

    /**
     * Current state of the button
     */
    virtual bool current() const;
    /**
     * returns true when single click was detected
     * Resets it´s internal state s a second call to this function will return false
     */
    virtual bool isSingle() const;
    /**
     * Returns true when double click was detected
     * Resets it´s internal state s a second call to this function will return false
     */
    virtual bool isDouble() const;
    /**
     * Returns true when double click was detected
     * Resets it´s internal state s a second call to this function will return false
     */
    virtual bool isLong() const;
    /**
     * Returns true when edge up was detected
     * Resets it´s internal state s a second call to this function will return false
     */
    virtual bool isEdgeUp() const;
    /**
     * Returns true when edge down was detected
     * Resets it´s internal state s a second call to this function will return false
     */
    virtual bool isEdgeDown() const;
    /**
     * Reset the internal state, but not the button states
     */
    virtual void reset() const;
    /**
     * Reset the button states, but keep the current state based on it´s last measurement
     */
    virtual void resetButtons() const;
    /**
     * Return the internal representation of the system
     */
    std::bitset<32> intern() const {
        return std::bitset<32>(m_status.m_status32);
    }
    /**
     * Return the internal representation of the buttom states
     */
    std::bitset<6> presses() const {
        return std::bitset<6>(m_value);
    }
};
