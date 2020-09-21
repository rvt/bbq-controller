#include "NTCSensor.h"
#include <math.h>
#include <Arduino.h>


NTCSensor::NTCSensor(int8_t p_pin, float p_offset, float p_r1, float p_c1, float p_c2, float p_c3) :
    TemperatureSensor(),
    m_pin(p_pin),
    m_r1(p_r1),
    m_offset(p_offset),
    m_c1(p_c1),
    m_c2(p_c2),
    m_c3(p_c3),
    m_lastTemp(0.0f),
    m_recover(0),
    m_state(READY) {

#if defined(ESP32)
    analogReadResolution(12);
    analogSetSamples(4);
    analogSetClockDiv(1);
    analogSetPinAttenuation(m_pin, ADC_0db);
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
            m_state = READY;
            float Vo = adcEnd(m_pin);
            float R2 = m_r1 * (4096.0f / Vo - 1.0f);
            float logR2 = log(R2);
            float degreesC = (1.0f / (m_c1 + m_c2 * logR2 + m_c3 * logR2 * logR2 * logR2));
            degreesC = degreesC - 273.15f;
            // T = (T * 9.0f) / 5.0f + 32.0f;
            // float farenheit = ((degreesC * 9) / 5) + 32.
            // Filter
            m_lastTemp = m_lastTemp + (degreesC - m_lastTemp) * 0.1f;
            break;
    }

}
#elif defined(ESP8266)
// This portion of code was not tested on ESP2866
void NTCSensor::handle() {
    float Vo = analogRead(A0);
    float R2 = m_r1 * (1023.0 / Vo - 1.0);

    
    float logR2 = log(R2);
    float degreesC = (1.0f / (m_c1 + m_c2 * logR2 + m_c3 * logR2 * logR2 * logR2));
    degreesC = degreesC - 273.15f;
    // T = (T * 9.0f) / 5.0f + 32.0f;
    // float farenheit = ((degreesC * 9) / 5) + 32.
    // Filter
    m_lastTemp = m_lastTemp + (degreesC - m_lastTemp) * 0.1f;
}
#endif