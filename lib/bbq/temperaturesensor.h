#pragma once

class TemperatureSensor {
public:
    virtual float get() const = 0;
    virtual void handle() {
    };
};

class DummyTemperatureSensor : public TemperatureSensor {
public:
    virtual float get() const {
        return 0.f;
    }
    virtual void handle() {
    };
};
