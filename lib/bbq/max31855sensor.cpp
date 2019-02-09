#include "max31855sensor.h"


MAX31855sensor::MAX31855sensor(MAX31855* p_MAX31855) :
    TemperatureSensor(),
    m_MAX31855(p_MAX31855),
    m_lastTemp(-1.0) {
}

void MAX31855sensor::handle() {
    float temperature = m_MAX31855->readThermocouple(CELSIUS);

    switch ((int) temperature) {
        case FAULT_OPEN:
            Serial.print("FAULT_OPEN");
            return;
        case FAULT_SHORT_GND:
            Serial.print("FAULT_SHORT_GND");
            return;
        case FAULT_SHORT_VCC:
            Serial.print("FAULT_SHORT_VCC");
            return;
        case NO_MAX31855:
            Serial.print("NO_MAX31855");
            return;
    }
    
    m_lastTemp = m_lastTemp + (temperature - m_lastTemp) * 0.1f;
}

float MAX31855sensor::get() const {
    return m_lastTemp;
}
