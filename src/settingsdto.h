#pragma once

#include <stdint.h>
#include <array>

#include <bbqfanonly.h>

struct SettingsDTOData  {
    float setPoint = 20;

    float  fan_startPwm1 = 20;   // PWM value (0..100%) where the fan actually starts rotating
    int8_t lid_open_fan_speed = FAN_SPEED_LID_OPEN; // fan speed when lif is open. -1:keep as what is given: 0..100% force a speed

    std::array<float, 4> fan_low  = FAN_LOW_DEFAULT;
    std::array<float, 4> fan_medium  = FAN_MEDIUM_DEFAULT;
    std::array<float, 4> fan_high = FAN_HIGH_DEFAULT;

    std::array<float, 2> temp_error_low = TEMP_ERROR_LOW_DEFAULT;
    std::array<float, 4> temp_error_medium  = TEMP_ERROR_MEDIUM_DEFAULT;
    std::array<float, 4> temp_error_hight = TEMP_ERROR_HIGH_DEFAULT;

    std::array<float, 2> temp_change_slow = TEMP_CHANGE_LOW_DEFAULT;
    std::array<float, 4> temp_change_medium = TEMP_CHANGE_MEDIUM_DEFAULT;
    std::array<float, 4> temp_change_fast = TEMP_CHANGE_FAST_DEFAULT;

    bool operator==(const  SettingsDTOData& rhs) {
        return
            setPoint == rhs.setPoint &&
            fan_startPwm1 == rhs.fan_startPwm1 &&
            lid_open_fan_speed == rhs.lid_open_fan_speed &&
            fan_low == rhs.fan_low &&
            fan_medium == rhs.fan_medium &&
            fan_high == rhs.fan_high &&
            temp_error_low == rhs.temp_error_low &&
            temp_error_medium == rhs.temp_error_medium &&
            temp_error_hight == rhs.temp_error_hight &&
            temp_change_slow == rhs.temp_change_slow &&
            temp_change_medium == rhs.temp_change_medium &&
            temp_change_fast == rhs.temp_change_fast;
    }

    bool operator!=(const SettingsDTOData& rhs) {
        return !(*this == rhs);
    }
};

class SettingsDTO final {
private:
    SettingsDTOData m_data;
    SettingsDTOData l_data;
public:


public:
    SettingsDTO(const SettingsDTOData& data) :
        m_data(data),
        l_data(data) {
    }
    SettingsDTO() {
    }

    bool modified()  {
        return m_data != l_data;
    }

    SettingsDTOData* data() {
        return &m_data;
    }

    void data(const SettingsDTOData& data) {
        memcpy(&m_data, &data, sizeof(data));
    }

    void reset() {
        memcpy(&l_data, &m_data, sizeof(m_data));
    }

    std::string getConfigString() {
        return makeString("sp=%.1f"
                          " fs1=%.0f"
                          " lof=%i"
                          " fl1=%.1f,%.1f,%.1f,%.1f"
                          " fm1=%.1f,%.1f,%.1f,%.1f"
                          " fh1=%.1f,%.1f,%.1f,%.1f"
                          " tel=%.1f,%.1f"
                          " tem=%.1f,%.1f,%.1f,%.1f"
                          " teh=%.1f,%.1f,%.1f,%.1f"
                          " tcs=%.1f,%.1f"
                          " tcm=%.1f,%.1f,%.1f,%.1f"
                          " tcf=%.1f,%.1f,%.1f,%.1f",
                          m_data.setPoint,
                          m_data.fan_startPwm1,
                          m_data.lid_open_fan_speed,
                          m_data.fan_low[0], m_data.fan_low[1], m_data.fan_low[2], m_data.fan_low[3],
                          m_data.fan_medium[0], m_data.fan_medium[1], m_data.fan_medium[2], m_data.fan_medium[3],
                          m_data.fan_high[0], m_data.fan_high[1], m_data.fan_high[2], m_data.fan_high[3],
                          m_data.temp_error_low[0], m_data.temp_error_low[1],
                          m_data.temp_error_medium[0], m_data.temp_error_medium[1], m_data.temp_error_medium[2], m_data.temp_error_medium[3],
                          m_data.temp_error_hight[0], m_data.temp_error_hight[1], m_data.temp_error_hight[2], m_data.temp_error_hight[3],
                          m_data.temp_change_slow[0], m_data.temp_change_slow[1],
                          m_data.temp_change_medium[0], m_data.temp_change_medium[1], m_data.temp_change_medium[2], m_data.temp_change_medium[3],
                          m_data.temp_change_fast[0], m_data.temp_change_fast[1], m_data.temp_change_fast[2], m_data.temp_change_fast[3]
                         );
    }
};
