#pragma once

class DisplayController {
public:
    virtual int32_t handle() = 0;
    virtual void init() = 0;
};

