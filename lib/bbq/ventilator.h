#pragma once

class Ventilator {
public:
    virtual void speed(float speed) = 0;
    virtual float speed() = 0;
};
