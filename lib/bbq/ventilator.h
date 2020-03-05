#pragma once

#include <utils.h>

class Ventilator {
private:
    float m_overrideSetting;
    float m_speed;
    bool m_on;
public:
    Ventilator() :
        m_overrideSetting(-1.0f),
        m_speed(0.0f),
        m_on(true) {
    }

    /**
     * Set normal speed operation
     * p_speed: Dutiy cycle 0..100%
     */
    void speed(float p_speed) {
        m_speed = between(p_speed, 0.0f, 100.0f);
        setVentilator(speed());
    }

    /**
     * Returns current required speed
     * Value is guaranteed to be 0..100
     */
    float speed() const {
        return m_on ? (isOverride() ? m_overrideSetting : m_speed) : 0.0f;
    }

    void setOn(bool on) {
        m_on = on;
    }

    bool isOn() const {
        return m_on;
    }

    /**
     * Returns true if the ventilator is in override mode
     */
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

    virtual void handle() = 0;

private:
    virtual void setVentilator(const float dutyCucle) = 0;
};
