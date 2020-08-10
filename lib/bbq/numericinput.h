#pragma once

class NumericInput {
public:
    virtual float value() const = 0;
    virtual void value(float value) = 0;
    virtual void handle() = 0;

};
