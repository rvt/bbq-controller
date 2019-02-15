#pragma once

#include <utils.h>

class Ventilator {
private:
    float m_overrideSetting;
    float m_speed;
public:
    Ventilator() : m_overrideSetting(-1.0f), m_speed(0.0f) {
    }

    /**
     * Set normal speed operation
     */
    void speed(float p_speed) {
        m_speed = between(p_speed, 0.0f, 100.0f);
        setVentilator();
    }

    /**
     * Returns current required speed
     * Value is guaranteed to be 0..100
     */
    float speed() const {
        return isOverride() ? m_overrideSetting : m_speed;
    }

    bool isOverride() const {
        return m_overrideSetting >= 0.0;
    }
    /**
     * Set override speed
     */
    void speedOverride(float p_speed) {
        m_overrideSetting = between(p_speed, -1.0f, 100.0f);
    }
    float speedOverride() const {
        return m_overrideSetting;
    }

private:
    virtual float setVentilator() = 0;

};
