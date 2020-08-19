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
constexpr uint16_t PWM_RANGE = (1 << PWM_RESOLUTION) - 1;
constexpr uint16_t PWM_FREQUENCY = 10000;
#elif defined(ESP32)
constexpr uint8_t PWM_RESOLUTION = 8;
constexpr uint16_t PWM_RANGE = (1 << PWM_RESOLUTION) - 1;
constexpr uint16_t PWM_FREQUENCY = 25000;
#else
#error Must define ESP8266 or ESP32
#endif

PWMVentilator::PWMVentilator(uint8_t p_pin, uint8_t p_pwmStart) : PWMVentilator(p_pin, p_pwmStart, 0) {

}
PWMVentilator::PWMVentilator(uint8_t p_pin, uint8_t p_pwmStart, uint8_t p_pwmChannel) : PWMVentilator(p_pin, p_pwmStart, 30, p_pwmChannel) {

}

PWMVentilator::PWMVentilator(uint8_t p_pin, uint8_t p_pwmStart, uint8_t p_pwmMinimum, uint8_t p_pwmChannel) :
    Ventilator(),
    m_pin(p_pin),
    m_pwmStart(percentmap(p_pwmStart, PWM_RANGE)),
    m_pwmMinimum(percentmap(p_pwmMinimum, PWM_RANGE)),
    m_prevPwmValue(0),
    m_pwmChannel(p_pwmChannel)  {
#if defined(ESP8266)
    analogWriteRange(PWM_RANGE);
    analogWriteFreq(PWM_FREQUENCY);
    pinMode(p_pin, OUTPUT);
#pragma message "Configuring PWM pin ESP8266 "
#elif defined(ESP32)
    ledcAttachPin(p_pin, m_pwmChannel);
    ledcSetup(m_pwmChannel, PWM_FREQUENCY, PWM_RESOLUTION);
#endif
}


void PWMVentilator::setVentilator(float dutyCycle) {
    uint16_t pwmValue;
    bool doWait = false;

    dutyCycle = between(dutyCycle, 0.f, 200.f);

    // any speed below 1 is considered off
    if (dutyCycle >= 1.f && m_prevPwmValue < 1.0f) {
        // When the fan was off and turns on give it a little 'kick'
        // Currently a hack, need to find a better way!
        // The delay should only happen when turning on
        pwmValue = m_pwmStart;
        doWait = true;
    } else if (dutyCycle < 1.f) {
        pwmValue = 0;
    } else {
        pwmValue = fmap(dutyCycle, 0.f, 100.f, m_pwmMinimum, PWM_RANGE);
    }

#if defined(ESP8266)
    analogWrite(m_pin,  between(pwmValue, (uint16_t)0, PWM_RANGE));
#elif defined(ESP32)
    ledcWrite(m_pwmChannel, pwmValue);
#endif

    if (doWait) {
        delay(250);
    }

    m_prevPwmValue = dutyCycle;
}
