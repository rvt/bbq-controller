#pragma once

#include "temperaturesensor.h"
#include "ventilator.h"


class MockedTemperature : public TemperatureSensor {
private:
    float mTemperature;
public:
    MockedTemperature(float pTemperature) : mTemperature(pTemperature) {
    }
    virtual float get() {
        return mTemperature;
    }
    void set(float pTemperature) {
        mTemperature = pTemperature;
    }
};

class MockedFan : public Ventilator {
private:
    float mSpeed;
public:
    MockedFan() : mSpeed(0.0f) {
    }

    virtual void speed(float pSpeed) {
        mSpeed = pSpeed+1;
    }

    virtual float speed() {
        return mSpeed;
    }
};
