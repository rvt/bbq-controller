#pragma once

#include <memory>
#include <array>

#include "bbq.h"
#include "temperaturesensor.h"
#include "ventilator.h"

#include <Fuzzy.h>
#include <makestring.h>
#include <cstdlib>
#include <algorithm>

struct BBQFanOnlyConfig {
    int8_t fan_speed_lid_open = 0;
    std::array<float, 4> fan_low  = std::array<float, 4> { {0, 25, 25, 50} };
    std::array<float, 4> fan_medium  = std::array<float, 4> { {25, 50, 50, 75} };
    std::array<float, 4> fan_high = std::array<float, 4> { {50, 75, 100, 100} };

    std::array<float, 2> temp_error_low = std::array<float, 2> { {0, 5} };
    std::array<float, 4> temp_error_medium  = std::array<float, 4> { {0, 10, 10, 25} };
    std::array<float, 4> temp_error_hight = std::array<float, 4> { {10, 100, 200, 200} };

    std::array<float, 2> temp_change_slow = std::array<float, 2> { {0, 0.5} };
    std::array<float, 4> temp_change_medium = std::array<float, 4> { {0, 1, 1, 2.5} };
    std::array<float, 4> temp_change_fast = std::array<float, 4> { {1, 2.5, 20, 20} };
};

class BBQFanOnly : public BBQ {
private:
    std::shared_ptr<TemperatureSensor> m_tempSensor;
    std::shared_ptr<Ventilator> m_fan;
    Fuzzy* m_fuzzy;
    float m_setPoint;        // Setpoint
    float m_tempLastError;   // Last temperature error input
    float m_fanCurrentSpeed; // current fan speed
    float m_tempLast;        // Temperature that is bassed through a filter
    float m_lastTempChange;      // Keep
    bool m_lidOpenTriggered;
    BBQFanOnlyConfig m_config;
public:
    BBQFanOnly(std::shared_ptr<TemperatureSensor> pTempSensor,
               std::shared_ptr<Ventilator> pFan);
    virtual ~BBQFanOnly();
    /**
     * Very important, call this once in 5 seconds
     */
    virtual void handle();
    virtual void setPoint(float temperature);
    virtual float setPoint() const;
    virtual bool lowCharcoal();
    virtual bool lidOpen();

    // Fuzzy inputs monitoring
    float tempChangeInput() const;
    float lastErrorInput() const;
    bool ruleFired(uint8_t i);
    void config(const BBQFanOnlyConfig& p_config);
    BBQFanOnlyConfig config() const;

    /**
     * Cretae a fuzzy set from a vector of floats
     * Warning don´t change to &data !!
     */
    template<std::size_t SIZE>
    static FuzzySet* fuzzyFromVector(std::array<float, SIZE>& data, bool flipped) {
        std::array<float, SIZE> cpy = data;

        // Only flip and negate when we have a dataset of 4 items
        if (flipped && (SIZE == 4)) {
            std::reverse(cpy.begin(), cpy.end());
            std::for_each(cpy.begin(), cpy.end(), [](float & el) {
                el *= -1;
            });
        }

        FuzzySet* fs;

        if (SIZE == 2) {
            fs = new FuzzySet(-cpy[1], -cpy[0], cpy[0], cpy[1]);
        } else if (SIZE == 4) {
            fs = new FuzzySet(cpy[0], cpy[1], cpy[2], cpy[3]);
        } else {
            fs = new FuzzySet(0, 0, 0, 0);
        }

        return fs;
    }

    void init();
    virtual const char* name() const {
        return "fuzzy";
    }

private:
    FuzzyRule* joinSingle(int rule, FuzzySet* fi, FuzzySet* fo);
    FuzzyRule* joinSingleAND(int rule, FuzzySet* fi1, FuzzySet* fi2, FuzzySet* fo);

};
