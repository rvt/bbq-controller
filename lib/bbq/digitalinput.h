#pragma once

class DigitalInput {
public:
    virtual bool current() const = 0;
    virtual bool isSingle() const = 0;
    virtual bool isDouble() const = 0;
    virtual bool isLong() const = 0;
};
