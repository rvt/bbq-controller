#include "pwmventilator.h"
#include <utils.h>

#ifndef UNIT_TEST
#include <Arduino.h>
#else
extern "C" uint32_t millis();
extern "C" void analogWrite(uint8_t pin, int16_t value);
extern "C" void analogWriteRange(int16_t value);
extern "C" void analogWriteFreq(int16_t value);
extern "C" uint32_t pinMode(uint8_t, uint8_t);
extern "C" void delay(uint16_t);
#define OUTPUT 0
#endif

#if defined(ESP8266)        
constexpr uint8_t PWM_RESOLUTION = 8;
constexpr uint16_t PWM_RANGE = 1 << PWM_RESOLUTION -1;
constexpr uint16_t PWM_FREQUENCY = 10000;
#elif defined(ESP32)
constexpr uint8_t PWM_RESOLUTION = 8;
constexpr uint16_t PWM_RANGE = 1 << PWM_RESOLUTION - 1;
constexpr uint16_t PWM_FREQUENCY = 25000;
#else
#endif

PWMVentilator::PWMVentilator(uint8_t p_pin, uint8_t p_pwmStart) : PWMVentilator(p_pin, p_pwmStart, 0) {

}

PWMVentilator::PWMVentilator(uint8_t p_pin, uint8_t p_pwmStart, uint8_t p_pwmChannel) :
    Ventilator(),
    m_pin(p_pin),
    m_pwmStart(percentmap(p_pwmStart, PWM_RANGE)),
    m_prevPwmValue(0),
    m_pwmChannel(p_pwmChannel)  {
#if defined(ESP8266)        
    analogWriteRange(PWM_RANGE);
    analogWriteFreq(PWM_FREQUENCY);
#elif defined(ESP32)
    ledcSetup(p_pwmChannel, PWM_FREQUENCY, PWM_RESOLUTION);
    ledcAttachPin(p_pin, p_pwmChannel);
#endif
    pinMode(p_pin, OUTPUT);
    // Ensure value is in between right values
    m_pwmStart = between(p_pwmStart, (uint8_t)0, (uint8_t)100);
}


void PWMVentilator::setVentilator(float dutyCycle) {
    uint16_t pwmValue;

    // any speed below 1 is considered off
    if (dutyCycle < 1) {
        pwmValue = 0;
    } else {
        pwmValue = fmap(dutyCycle, 0.f, 100.f, m_pwmStart, PWM_RANGE);

        // When the fan was off and turns on give it a little 'kick'
        // Currently a hack, need to find a better way!
        // The delay should only happen when turning on
        if (m_prevPwmValue < 1.0f) {

#if defined(ESP8266)
            analogWrite(m_pin, PWM_RANGE);
#else
            ledcWrite(m_pwmChannel, PWM_RANGE);
#endif
            delay(250);
        }
    }

#if defined(ESP8266)
    analogWrite(m_pin, pwmValue);
#else
    ledcWrite(m_pwmChannel, pwmValue);
#endif

    m_prevPwmValue = dutyCycle;
}
