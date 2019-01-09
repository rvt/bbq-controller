#pragma once

#include <stdint.h>


/**
 * Read analog in on A0 pin on ESP8266
 * Will filter the result and map to min/max values
 */
class AnalogIn {
private:
    float m_min;
    float m_max;
    float m_alpha;
    float m_value;
public:
    AnalogIn(float p_min, float p_max);
    void handle();
    float value() const;

};
