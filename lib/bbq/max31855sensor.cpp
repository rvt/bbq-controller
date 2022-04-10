#include "max31855sensor.h"

MAX31855sensor::MAX31855sensor(MAX31855* p_MAX31855) :
    TemperatureSensor(),
    m_MAX31855(p_MAX31855),
    m_lastTemp(-1.0),
    m_faultCode(0),
    m_temp_sum(0.0f),
    m_samplecount(0) {
}

void MAX31855sensor::handle() {
    uint8_t fault = m_MAX31855->read();
    float measured = m_MAX31855->getTemperature();
    //float measured = m_MAX31855->getInternal();


    if (fault != 0) {
        m_faultCode = m_MAX31855->getStatus();
        Serial.println (m_faultCode);
    } else {
        constexpr uint8_t SAMPLES = 5; // at 50Hz about 2 samples per sec
        m_temp_sum += measured;
        m_samplecount += 1;
//      Serial.print ("measured:");
//      Serial.println (measured);

        if (m_samplecount >= SAMPLES) {
            m_lastTemp = m_lastTemp + ((m_temp_sum / m_samplecount) - m_lastTemp) * 0.25f;
            m_temp_sum = 0.0f;
            m_samplecount = 0;
        }
    }
}

uint16_t MAX31855sensor::faultCode() const {
    return m_faultCode;
}

float MAX31855sensor::get() const {
    return m_lastTemp;
}
