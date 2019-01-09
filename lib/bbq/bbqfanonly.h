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
    float temp_alpha = .1f;         // Lid Open Filter Alpha Fan filter

    std::array<float, 4> fan_low  { {0, 0, 0, 50} };
    std::array<float, 4> fan_medium  { {25, 50, 50, 75} };
    std::array<float, 4> fan_high  { {50, 100, 100, 100} };

    std::array<float, 2> temp_error_low  { {0, 10} };
    std::array<float, 4> temp_error_medium  { {0, 15, 15, 30} };
    std::array<float, 4> temp_error_hight { {15, 200, 200, 200} };

    std::array<float, 4> temp_change_fast { {10, 20, 20, 30} };

};

class BBQFanOnly : public BBQ {
private:
    std::shared_ptr<TemperatureSensor> m_tempSensor;
    std::shared_ptr<Ventilator> m_fan;
    Fuzzy* m_fuzzy;
    float m_setPoint;       // Setpoint
    float m_tempLastError;  // Last temperature error input
    float m_fanCurrentSpeed;            // current fan speed
    float m_tempFiltered;   // Temperature that is bassed through a filter
    float m_deltaError;  // Delta error inout
    float m_tempDropFiltered;    // Temperature Drop Input but filtered using alpha
    BBQFanOnlyConfig m_config;
public:
    BBQFanOnly(std::shared_ptr<TemperatureSensor> pTempSensor,
               std::shared_ptr<Ventilator> pFan);
    virtual ~BBQFanOnly();
    virtual void handle();
    virtual void setPoint(float temperature);
    virtual float setPoint() const;
    virtual bool lowCharcoal();
    virtual bool lidOpen();

    // Fuzzy inputs monitoring
    float deltaErrorInput() const;
    float tempDropFilteredInput() const;
    float lastErrorInput() const;
    void config(const BBQFanOnlyConfig& p_config);
    BBQFanOnlyConfig config() const;

    /**
     * Cretae a fuzzy set from a vector of floats
     * Warning don´t change to &data !!
     */
    template<std::size_t SIZE>    
    static FuzzySet* fuzzyFromVector(std::array<float, SIZE> &data, bool flipped) {
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

private:
    FuzzyRule* joinSingle(int rule, FuzzySet* fi, FuzzySet* fo);
    FuzzyRule* joinSingleAND(int rule, FuzzySet* fi1, FuzzySet* fi2, FuzzySet* fo);

};
