#include "stefanspwmventilator.h"
#include <utils.h>
#include <math.h>

#ifndef UNIT_TEST
#include <Arduino.h>
extern "C" {
#include "pwm.h"
}
#else
#define OUTPUT 0
#endif

constexpr uint32_t STEFANS_PWM_PERIOD1K = 5000; // 1Khz PWM
constexpr uint32_t STEFANS_PWM_PERIOD = STEFANS_PWM_PERIOD1K / 15;
constexpr uint32_t STEFANS_PWM_PERIOD10 = STEFANS_PWM_PERIOD / 10; // 10 % of STEFANS_PWM_PERIOD

StefansPWMVentilator::StefansPWMVentilator(uint8_t p_pin, uint8_t p_pwmStart) :
    Ventilator(),
    m_pin(p_pin),
    m_pwmStart(p_pwmStart),
    m_pwmValue(0.0f),
    m_currentPwmValue(0.0f),
    m_lastMillis(0) {

#define TOTALPINS 9
    const uint32_t all_io_info[TOTALPINS][3] = {
        {PERIPHS_IO_MUX_GPIO5_U, FUNC_GPIO5,   5}, // D1
        {PERIPHS_IO_MUX_GPIO4_U, FUNC_GPIO4,   4}, // D2
        {PERIPHS_IO_MUX_U0RXD_U, FUNC_GPIO3,   3}, // DXX
        {PERIPHS_IO_MUX_GPIO0_U, FUNC_GPIO0,   0}, // D3
        {PERIPHS_IO_MUX_GPIO2_U, FUNC_GPIO2,   2}, // D4
        {PERIPHS_IO_MUX_MTMS_U,  FUNC_GPIO14, 14}, // D5
        {PERIPHS_IO_MUX_MTDI_U,  FUNC_GPIO12, 12}, // D6
        {PERIPHS_IO_MUX_MTCK_U,  FUNC_GPIO13, 13}, // D7
        {PERIPHS_IO_MUX_MTDO_U,  FUNC_GPIO15, 15}, // D8
    };
    // Find the section to be copied for this pin configuration
    auto pushSetup = [&all_io_info](uint32_t* io_config, const uint8_t pin) {
        for (int8_t i = 0; i < TOTALPINS; i++) {
            if ((uint8_t)all_io_info[i][2] == pin) {
                memcpy(io_config, all_io_info[i], sizeof(all_io_info[i]));
                pinMode(pin, OUTPUT);
                digitalWrite(pin, LOW);
            }
        }
    };
    // Copy settings for the channels
    pushSetup(io_info[0], m_pin);

    uint32_t pwm_duty_init[] = {0};
    pwm_init(STEFANS_PWM_PERIOD, pwm_duty_init, 1, io_info);

    // Ensure value is in between right values
    m_pwmStart = between(p_pwmStart, (uint8_t)0, (uint8_t)100);
}

void StefansPWMVentilator::setVentilator(const float dutyCycle) {
    if (m_pwmValue < 0.1f && dutyCycle > 1.0f) {
        m_currentPwmValue = 50.0f;
    }

    m_pwmValue = dutyCycle;

}

void StefansPWMVentilator::handle(const uint32_t millis) {

    if (fabs(m_currentPwmValue - m_pwmValue) > 1.0) {
        m_currentPwmValue = m_currentPwmValue + (m_pwmValue - m_currentPwmValue) * 0.1f;

        float withStart ;

        if (m_currentPwmValue > 1.0f) {
            withStart = m_currentPwmValue * (100.f - m_pwmStart) / 100.0 + m_pwmStart;
        } else {
            withStart = 0;
        }

        int32_t pwmDuty = percentmap(withStart, STEFANS_PWM_PERIOD);

        if (millis - m_lastMillis > 1000) {
            m_lastMillis = millis;
            Serial.print("Duty : ");
            Serial.print(pwmDuty);
            Serial.print(" ");
            Serial.println(millis);
        }

        // Not sure why but when I get very close to 100% doing PWM
        // With PWM_USE_NMI set to 1 will crash the controller 
        if (pwmDuty + STEFANS_PWM_PERIOD10 < STEFANS_PWM_PERIOD) {
            pwm_set_duty(pwmDuty, 0);
            pwm_start();
        }

    }

};
