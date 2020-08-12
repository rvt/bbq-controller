#include "max31865sensor.h"

extern HardwareSerial Serial;

MAX31865sensor::MAX31865sensor(int8_t spi_cs, int8_t spi_mosi, int8_t spi_miso, int8_t spi_clk, float p_RNominal, float p_Rref) :
    TemperatureSensor(),
    Adafruit_MAX31865(spi_cs, spi_mosi, spi_miso, spi_clk),
    m_RNominal(p_RNominal),
    m_Rref(p_Rref),
    m_lastTemp(20.0),
    m_lastMillis(0),
    m_commState(0) {
}

void MAX31865sensor::handle() {
    if (m_commState == 0) {
        clearFault();
        enableBias(true);
        m_commState++;
        m_lastMillis = millis();
    } else if (m_commState == 1 && (millis() - m_lastMillis) > 10) {
        uint8_t t = readRegister8(MAX31856_CONFIG_REG);
        t |= MAX31856_CONFIG_1SHOT;
        writeRegister8(MAX31856_CONFIG_REG, t);
        m_commState++;
        m_lastMillis = millis();
    } else if (m_commState == 2 && (millis() - m_lastMillis) > 65) {
        uint16_t rtd = readRegister16(MAX31856_RTDMSB_REG);
        enableBias(false);  // to lessen sensor self-heating
        m_commState = 0;
        uint8_t fault = readFault();

        if (fault != 0) {
            Serial.print("Fault 0x");
            Serial.println(fault, HEX);

            if (fault & MAX31865_FAULT_HIGHTHRESH) {
                Serial.println(F("RTD High Threshold"));
            }

            if (fault & MAX31865_FAULT_LOWTHRESH) {
                Serial.println(F("RTD Low Threshold"));
            }

            if (fault & MAX31865_FAULT_REFINLOW) {
                Serial.println(F("REFIN- > 0.85 x Bias"));
            }

            if (fault & MAX31865_FAULT_REFINHIGH) {
                Serial.println(F("REFIN- < 0.85 x Bias - FORCE- open"));
            }

            if (fault & MAX31865_FAULT_RTDINLOW) {
                Serial.println(F("RTDIN- < 0.85 x Bias - FORCE- open"));
            }

            if (fault & MAX31865_FAULT_OVUV) {
                Serial.println(F("Under/Over voltage"));
            }

            clearFault();
            return;
        }

        // Use uint16_t (ohms * 100) since it matches data type in lookup table.
        uint32_t dummy = ((uint32_t)rtd) * 100 * ((uint32_t) floor(m_Rref)) ;
        dummy >>= 16 ;
        uint16_t ohmsx100 = (uint16_t)(dummy & 0xFFFF) ;

        // or use exact ohms floating point value.
        //float ohms = (float)(ohmsx100 / 100) + ((float)(ohmsx100 % 100) / 100.0) ;

        m_lastTemp = m_lastTemp + (m_PT100.celsius(ohmsx100) - m_lastTemp) * 0.1f;
    } else {
        // Failsave
        m_commState = 0;
    }
}

float MAX31865sensor::get() const {
    return floor(m_lastTemp * 10.0f + 0.5f) / 10.0f;
}
