#pragma once

class BBQ {

public:
    virtual void handle() = 0;
    virtual void setPoint(float temperature) = 0;
    virtual float setPoint() const = 0;
    virtual bool lowCharcoal() = 0;
    virtual bool lidOpen() = 0;
};
