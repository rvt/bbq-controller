#pragma once

#include "temperaturesensor.h"
#include <Adafruit_MAX31865.h>
#include <pt100rtd.h>

/**
 * non blocking version of Adafruit_MAX31865
 * The original Library used delay to wait for conversion completed
 * This version will use a small state machine to keep track of timing and
 * will ensure no delays when calling handle()
 */
class MAX31865sensor : public TemperatureSensor, public Adafruit_MAX31865 {
private:
    const float m_RNominal;
    const float m_Rref;
    float m_lastTemp;
    uint32_t m_lastMillis;
    uint8_t m_commState;
    pt100rtd m_PT100;
public:
    MAX31865sensor(int8_t spi_cs, int8_t spi_mosi, int8_t spi_miso, int8_t spi_clk, float p_RNominal, float p_Rref);
    virtual float get() const;
    virtual void handle();
};
