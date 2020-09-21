#include "max31855sensor.h"
#include <Adafruit_MAX31855.h>

#define MAX31855_ERR_OC  0x1
#define MAX31855_ERR_GND 0x2
#define MAX31855_ERR_VCC 0x4

MAX31855sensor::MAX31855sensor(Adafruit_MAX31855* p_MAX31855) :
    TemperatureSensor(),
    m_MAX31855(p_MAX31855),
    m_lastTemp(-1.0) {
}

void MAX31855sensor::handle() {
    float temperature = m_MAX31855->readCelsius();

    uint8_t fault = m_MAX31855->readError();

    if (fault != 0) {
        // Serial.print("MAX31855\nFault 0x");
        // Serial.println(fault, HEX);
        // Serial.print(F("Thermocouple error(s): "));

        if (fault & MAX31855_ERR_OC) {
            Serial.print(F("[open circuit] "));
        }

        if (fault & MAX31855_ERR_GND) {
            Serial.print(F("[short to GND] "));
        }

        if (fault & MAX31855_ERR_VCC) {
            Serial.print(F("[short to VCC] "));
        }

        Serial.println();
        return;
    }

    m_lastTemp = m_lastTemp + (temperature - m_lastTemp) * 0.1f;
}

float MAX31855sensor::get() const {
    return m_lastTemp;
}
