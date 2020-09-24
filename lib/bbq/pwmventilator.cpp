#include "pwmventilator.h"
#include <utils.h>
#include <math.h>

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

// Time in ms we run the fan at FAN_KICK_DUTY when the fan comes out of full stop
constexpr uint16_t FAN_KICK_TIME = 500;
constexpr float FAN_KICK_DUTY = 75.0f;

#if defined(ESP8266)
constexpr uint8_t PWM_RESOLUTION = 8;
constexpr uint16_t PWM_RANGE = (uint16_t(1) << PWM_RESOLUTION) - 1;
constexpr uint16_t PWM_FREQUENCY = 10000;
#elif defined(ESP32)
constexpr uint16_t PWM_FREQUENCY = 19531;
#else
#error Must define ESP8266 or ESP32
#endif


// code and idea from 
constexpr float pwm_max_frequency_for_bit_depth(uint8_t bit_depth) { 
    return 80e6f / float(1 << bit_depth); 
}
constexpr float pwm_min_frequency_for_bit_depth(uint8_t bit_depth) {
  return 80e6f / ((((1 << 20) - 1) / 256.0f) * float(1 << bit_depth));
}

uint8_t pwm_bit_depth_for_frequency(float frequency) {
  for (uint8_t i = 20; i >= 1; i--) {
    const float min_frequency = pwm_min_frequency_for_bit_depth(i);
    const float max_frequency = pwm_max_frequency_for_bit_depth(i);
    if (min_frequency <= frequency && frequency <= max_frequency)
      return i;
  }
  return {};
}

uint32_t pwm_max_duty_for_frequency(uint16_t frequency) {
  return (uint32_t(1) << pwm_bit_depth_for_frequency(frequency)) - 1;
}


PWMVentilator::PWMVentilator(uint8_t p_pin, uint8_t p_dutyStart) : PWMVentilator(p_pin, p_dutyStart, 0) {

}
PWMVentilator::PWMVentilator(uint8_t p_pin, uint8_t p_dutyStart, uint8_t p_pwmChannel) :
    Ventilator(),
    m_pin(p_pin),
    m_prevPwmValue(0),
    m_kickTime(0),
    m_isKick(false),
#if defined(ESP8266)
    m_maxDuty(PWM_RANGE),
#elif defined(ESP32)
    m_maxDuty(pwm_max_duty_for_frequency(PWM_FREQUENCY)),
#endif
    m_dutyStart(p_dutyStart),
    m_pwmChannel(p_pwmChannel)  {
#if defined(ESP8266)
    analogWriteRange(PWM_RANGE);
    analogWriteFreq(PWM_FREQUENCY);
    pinMode(p_pin, OUTPUT);
#pragma message "Configuring PWM pin ESP8266 "
#elif defined(ESP32)
    ledcAttachPin(p_pin, m_pwmChannel);
    ledcSetup(m_pwmChannel, PWM_FREQUENCY, pwm_bit_depth_for_frequency(PWM_FREQUENCY));
#endif
}

PWMVentilator::~PWMVentilator() {
#if defined(ESP32)
    ledcDetachPin(m_pin);
#endif
}


void PWMVentilator::setVentilator(float dutyCycle) {
    const uint32_t currentMilis = millis();
    if (m_isKick && (currentMilis - m_kickTime < FAN_KICK_TIME)) return;
    m_isKick = false;
    dutyCycle = between(dutyCycle, 0.f, 100.f);

    // any speed below 1 is considered off
    if (dutyCycle >= 1.f && m_prevPwmValue < 1.0f) {
        // When the fan was off and turns on give it a little 'kick'
        // Currently a hack, need to find a better way!
        // The delay should only happen when turning on
        dutyCycle = FAN_KICK_DUTY;
        m_kickTime = millis();
        m_isKick = true;
    } else if (dutyCycle < 1.f) {
        dutyCycle = 0;
    } else {
        dutyCycle = fmap(dutyCycle, 0.f, 100.f, m_dutyStart, 100.f);
    }

    // From here dutyCycle is the 'real' duty cycle, not remapped
    const float duty_rounded = round((dutyCycle / 100.f) * m_maxDuty);
    const uint32_t pwmValue = dutyCycle = static_cast<uint32_t>(duty_rounded);    

#if defined(ESP8266)
    analogWrite(m_pin,  between((uint16_t)pwmValue, (uint16_t)0, PWM_RANGE));
#elif defined(ESP32)
    ledcWrite(m_pwmChannel, pwmValue);
#endif
    m_prevPwmValue = dutyCycle;
}
