#pragma once

#include <stdint.h>
#include <array>


struct SettingsDTOData  {
    float setPoint = 20;
    float temp_alpha = 0.1;
    std::array<float, 4> fan_low  { {0, 0, 0, 50} };
    std::array<float, 4> fan_medium  { {25, 50, 50, 75} };
    std::array<float, 4> fan_high  { {50, 100, 100, 100} };

    std::array<float, 2> temp_error_low  { {0, 10} };
    std::array<float, 4> temp_error_medium  { {0, 15, 15, 30} };
    std::array<float, 4> temp_error_hight { {15, 200, 200, 200} };

    std::array<float, 4> temp_change_fast { {10, 20, 20, 30} };

};

class SettingsDTO final {
private:
    SettingsDTOData m_data;
    SettingsDTOData l_data;
public:


public:
    bool modified() const {
        return memcmp(&m_data, &l_data, sizeof(m_data));
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
                          " ta=%.1f"
                          " fl1=%.1f,%.1f,%.1f,%.1f"
                          " fm1=%.1f,%.1f,%.1f,%.1f"
                          " fh1=%.1f,%.1f,%.1f,%.1f"
                          " tel=%.1f,%.1f"
                          " tem=%.1f,%.1f,%.1f,%.1f"
                          " teh=%.1f,%.1f,%.1f,%.1f"
                          " tcf=%.1f,%.1f,%.1f,%.1f",
                          m_data.setPoint,
                          m_data.temp_alpha,
                          m_data.fan_low[0], m_data.fan_low[1], m_data.fan_low[2], m_data.fan_low[3],
                          m_data.fan_medium[0], m_data.fan_medium[1], m_data.fan_medium[2], m_data.fan_medium[3],
                          m_data.fan_high[0], m_data.fan_high[1], m_data.fan_high[2], m_data.fan_high[3],
                          m_data.temp_error_low[0], m_data.temp_error_low[1],
                          m_data.temp_error_medium[0], m_data.temp_error_medium[1], m_data.temp_error_medium[2], m_data.temp_error_medium[3],
                          m_data.temp_error_hight[0], m_data.temp_error_hight[1], m_data.temp_error_hight[2], m_data.temp_error_hight[3],
                          m_data.temp_change_fast[0], m_data.temp_change_fast[1], m_data.temp_change_fast[2], m_data.temp_change_fast[3]
                         );
    }
};
