#include "max31865sensor.h"


MAX31865sensor::MAX31865sensor(Adafruit_MAX31865* p_MAX31865, float RNOMINAL, float RREF) :
    TemperatureSensor(),
    m_MAX31865(p_MAX31865),
    m_RNominal(RNOMINAL),
    m_Rref(RREF),
    m_lastTemp(-1.0) {
}

void MAX31865sensor::handle() {
    float tmp = m_MAX31865->temperature(m_RNominal, m_Rref);

    if (m_MAX31865->readFault() != 0) {
        return;
    }

    m_lastTemp = m_lastTemp + (tmp - m_lastTemp) * 0.1f;
}

float MAX31865sensor::get() const {
    return m_lastTemp;
}
