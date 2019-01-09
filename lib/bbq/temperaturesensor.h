#pragma once

class TemperatureSensor {
public:
    virtual float get() = 0;
    virtual void handle() {
    };
};
