#pragma once

#include "temperaturesensor.h"
#include "ventilator.h"


class MockedTemperature : public TemperatureSensor {
private:
    float mTemperature;
public:
    MockedTemperature(float pTemperature) : mTemperature(pTemperature) {
    }
    virtual float get() const {
        return mTemperature;
    }
    void set(float pTemperature) {
        mTemperature = pTemperature;
    }
};

class MockedFan : public Ventilator {
private:
    float m_speed;
public:
    MockedFan() : Ventilator(), m_speed(0.0) {
    }

    virtual float setVentilator() {
        m_speed = m_speed + (m_speed - speed()) * 0.1f;
        return m_speed;
    }
};
