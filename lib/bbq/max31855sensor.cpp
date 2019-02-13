#include "max31855sensor.h"
#include <Adafruit_MAX31855.h>

MAX31855sensor::MAX31855sensor(Adafruit_MAX31855* p_MAX31855) :
    TemperatureSensor(),
    m_MAX31855(p_MAX31855),
    m_lastTemp(-1.0) {
}

void MAX31855sensor::handle() {
    float temperature = m_MAX31855->readCelsius();

    if (m_MAX31855->readError() != 0) {
        Serial.println("MAX31855sensor: read error");
        return;
    }

    m_lastTemp = m_lastTemp + (temperature - m_lastTemp) * 0.1f;
}

float MAX31855sensor::get() const {
    return m_lastTemp;
}
