#pragma once

#include <stdint.h>
#include "temperaturesensor.h"

/**
 * NTC Temperature sensot with steinhart correction
 * TODO: SPlit into a ESP and nonESP version
 */
class NTCSensor : public TemperatureSensor {
private:
    const int8_t m_pin;
    const int8_t m_r1;
    const float m_offset;
    const float m_c1;
    const float m_c2;
    const float m_c3;
    float m_lastTemp;
    uint8_t m_recover; // Used as a fail save in case acdBuzy never goes true
    enum {
        READY,
        BUZY,
        DONE
    } m_state;
public:
    NTCSensor(int8_t p_pin, float p_offset, float p_r1, float p_c1, float p_c2, float p_c3);
    virtual float get() const;

    // Call handle a few times a second
    // To ensure filter works well
    virtual void handle();
};
