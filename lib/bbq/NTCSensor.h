#pragma once

#include <stdint.h>
#include "temperaturesensor.h"

/**
 * NTC Temperature sensot with steinhart correction
 * TODO: SPlit into a ESP and nonESP version
 *
 *  Connect NTC to 3.3V via a resistor to GND. Put the resister/NTC connectionto analog in.
 *
 * --- 3.3V
 *  |
 *  Resistor
 *  |
 *  -------- A0
 *  |
 *  NTC
 *  |
 * --- GND
 */
class NTCSensor : public TemperatureSensor {
private:
    const int8_t m_pin;
    const bool m_upDownStream;
    const float m_offset;
    const float m_alpha;
    const float m_r1;
    const float m_ka;
    const float m_kb;
    const float m_kc;
    float m_lastTemp;
    uint8_t m_recover; // Used as a fail save in case acdBuzy never goes true
    enum {
        READY,
        BUZY,
        DONE
    } m_state;
public:
    NTCSensor(int8_t p_pin, float p_offset, float p_r1, float p_ka, float p_kb, float p_kc);
    NTCSensor(int8_t p_pin, float p_offset, float p_alpha, float p_r1, float p_ka, float p_kb, float p_kc);
    // p_upDownStream=false DOWNSTREAM (NTC is at GND)
    // p_upDownStream=true UPSTREAM
    NTCSensor(int8_t p_pin, bool p_upDownStream, float p_offset, float p_alpha, float p_r1, float p_ka, float p_kb, float p_kc);
    virtual float get() const;

    // Call handle a few times a second
    // To ensure filter works well
    virtual void handle();

    /**
     * Calculate steinhart coefficients
     */
    static void calculateSteinhart(float r, float r1, float t1, float r2, float t2, float r3, float t3, float& ka, float& kb, float& kc);
};
