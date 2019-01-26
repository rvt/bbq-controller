#pragma once

class TemperatureSensor {
public:
    virtual float get() const = 0;
    virtual void handle() {
    };
};
