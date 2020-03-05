#include "onoffventilator.h"
#include <utils.h>

#ifndef UNIT_TEST
#include <Arduino.h>
#else
extern "C" uint32_t millis();
extern "C" void digitalWrite(uint8_t pin, int16_t value);
extern "C" uint32_t pinMode(uint8_t, uint8_t);
#define OUTPUT 0
#endif

#define PWM_RANGE  512
#define PWM_FREQUENCY 10000


OnOffVentilator::OnOffVentilator(uint8_t p_pin, uint32_t p_period) :
    Ventilator(),
    m_pin(p_pin),
    m_period(p_period) {
    pinMode(p_pin, OUTPUT);
}

void OnOffVentilator::setVentilator(const float dutyCycle) {


}

void OnOffVentilator::handle() {
    bool pinValue = 0;

    // any speed below 1 is considered off
    float dutyCycle = speed();

    if (dutyCycle >= 1) {
        const uint32_t ticksForOn = (m_period * dutyCycle) / 100.0f;
        const uint32_t periodCounter = millis() % m_period;
        pinValue = periodCounter < ticksForOn;
    }

    digitalWrite(m_pin, pinValue);
}

