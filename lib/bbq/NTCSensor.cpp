#include "NTCSensor.h"
#include <math.h>

#ifndef UNIT_TEST
#include <Arduino.h>
#else
extern "C" uint32_t analogRead(uint8_t);
#define INPUT_PULLUP 2
#define A0 0xa0
#endif

constexpr float ZERO_KELVIN = -273.15f;
NTCSensor::NTCSensor(int8_t p_pin, int16_t p_offset, float p_r1, float p_ka, float p_kb, float p_kc) :
    NTCSensor(p_pin, false, p_offset, 0.1f, p_r1, p_ka, p_kb, p_kc) {

}

NTCSensor::NTCSensor(int8_t p_pin, bool p_upDownStream, int16_t p_offset, float p_alpha, float p_r1, float p_ka, float p_kb, float p_kc) :
    TemperatureSensor(),
    m_pin(p_pin),
    m_upDownStream(p_upDownStream),
    m_offset(p_offset),
    m_alpha(p_alpha),
    m_r1(p_r1),
    m_ka(p_ka),
    m_kb(p_kb),
    m_kc(p_kc),
    m_lastTemp(0.0f),
    m_recover(0),
    m_state(READY) {

#if defined(ESP32)
    analogReadResolution(12);
    // These two do not work, possible because we take to much time between start and end??
    //    analogSetSamples(4);
    //    analogSetClockDiv(1);
    // End
    analogSetPinAttenuation(m_pin, ADC_11db); // ADC_11db to measure 0--3.3V
    adcAttachPin(m_pin);
#elif defined(ESP8266)
#endif
}

float NTCSensor::get() const {
    return m_lastTemp;
}

// Calculations based on https://create.arduino.cc/projecthub/iasonas-christoulakis/make-an-arduino-temperature-sensor-thermistor-tutorial-b26ed3
// With an additional filter
#if defined(ESP32)
void NTCSensor::handle() {
    switch (m_state) {
        case READY:
            adcStart(m_pin);
            m_state = BUZY;
            m_recover = 0;
            break;

        case BUZY:
            if (!adcBusy(m_pin)) {
                m_state = DONE;
            } else if (++m_recover == 255) {
                m_state = READY;
            }

            break;

        case DONE:
            // TODO: SEE https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/adc.html
            // Iw possible we can read the internal calibration of the reference voltage
            m_state = READY;
            float R2;
            if (m_upDownStream) {
                R2 = m_r1 * (4095.0f / (adcEnd(m_pin) + m_offset) - 1.0f);
            } else {
                R2 = m_r1 / (4095.0f / (adcEnd(m_pin) + m_offset) - 1.0f);
            }

            float logR2 = log(R2);
            float degreesC = m_ka + m_kb * logR2 + m_kc * logR2 * logR2 * logR2;
            degreesC = 1.0f / degreesC + ZERO_KELVIN;

            // T = (T * 9.0f) / 5.0f + 32.0f;
            // float farenheit = ((degreesC * 9) / 5) + 32.
            // Filter
            m_lastTemp = m_lastTemp + (degreesC - m_lastTemp) * m_alpha;
            break;
    }

}
#elif defined(ESP8266)
// This portion of code was not tested on ESP2866
void NTCSensor::handle() {
    float R2;
    if (m_upDownStream) {
        R2 = m_r1 * (1023.0f / (analogRead(A0) + m_offset) - 1.0f);
    } else {
        R2 = m_r1 / (1023.0f / (analogRead(A0) + m_offset) - 1.0f);
    }

    float logR2 = log(R2);
    float degreesC = m_ka + m_kb * logR2 + m_kc * logR2 * logR2 * logR2;
    degreesC = 1.0f / degreesC + ZERO_KELVIN;

    // T = (T * 9.0f) / 5.0f + 32.0f;
    // float farenheit = ((degreesC * 9) / 5) + 32.
    // Filter
    m_lastTemp = m_lastTemp + (degreesC - m_lastTemp) * m_alpha;
}
#endif

void NTCSensor::calculateSteinhart(float r, float r1, float t1, float r2, float t2, float r3, float t3, float& a, float& b, float& c) {
    float l1 = log(r1);
    float l2 = log(r2);
    float l3 = log(r3);

    float y1 = 1.0f / (t1 - ZERO_KELVIN);
    float y2 = 1.0f / (t2 - ZERO_KELVIN);
    float y3 = 1.0f / (t3 - ZERO_KELVIN);

    float g2 = (y2 - y1) / (l2 - l1);
    float g3 = (y3 - y1) / (l3 - l1);

    c = (g3 - g2) / (l3 - l2) * 1.0f / (l1 + l2 + l3);
    b = g2 - c * (l1 * l1 + l1 * l2 + l2 * l2);
    a = y1 - (b + l1 * l1 * c) * l1;
}

