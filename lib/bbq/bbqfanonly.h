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

constexpr uint8_t UPDATES_PER_SECOND = 1; // numnber of fuzzy calculations per second
constexpr uint8_t TEMPERATUR_DIFFERENCE_OVER_SEC = 5;

struct BBQFanOnlyConfig_org {
    int8_t fan_speed_lid_open = 0;
    std::array<float, 4> fan_lower  = std::array<float, 4> { {-10, -2, -2, 1} };
    std::array<float, 4> fan_steady  = std::array<float, 4> { {-2, 0, 0, 2} };
    std::array<float, 4> fan_higher = std::array<float, 4> { {1, 2, 2, 10} };

    std::array<float, 4> temp_error_low = std::array<float, 4> { {-5, 0, 0, 5} };
    std::array<float, 4> temp_error_medium  = std::array<float, 4> { {0, 10, 10, 20} };
    std::array<float, 4> temp_error_hight = std::array<float, 4> { {15, 30, 200, 200} };

    // Change per 5 seconds
    std::array<float, 4> temp_change_slow = std::array<float, 4> { {-0.1, 0, 0, 0.1} };
    std::array<float, 4> temp_change_medium = std::array<float, 4> { {0, 0.2, 0.2, 0.5} };
    std::array<float, 4> temp_change_fast = std::array<float, 4> { {0.2, 1, 20, 20} };
};


struct BBQFanOnlyConfig {
    int8_t fan_speed_lid_open = 0;
    std::array<float, 4> fan_lower  = std::array<float, 4> { {-5, -2, -2, 1} };
    std::array<float, 4> fan_steady  = std::array<float, 4> { {-2, 0, 0, 2} };
    std::array<float, 4> fan_higher = std::array<float, 4> { {1, 2, 2, 5} };

    std::array<float, 4> temp_error_low = std::array<float, 4> { {-5, 0, 0, 5} };
    std::array<float, 4> temp_error_medium  = std::array<float, 4> { {0, 20, 20, 40} };
    std::array<float, 4> temp_error_hight = std::array<float, 4> { {20, 75, 200, 200} };

    // Change per 5 seconds
    std::array<float, 4> temp_change_slow = std::array<float, 4> { {-2, 0, 0, 2} };
    std::array<float, 4> temp_change_medium = std::array<float, 4> { {0, 2, 2, 4} };
    std::array<float, 4> temp_change_fast = std::array<float, 4> { {2, 10, 20, 20} };
};

class BBQFanOnly : public BBQ {
private:
    std::shared_ptr<TemperatureSensor> m_tempSensor;
    std::shared_ptr<Ventilator> m_fan;
    Fuzzy* m_fuzzy;
    float m_setPoint;        // Setpoint
    BBQFanOnlyConfig m_config;
    long m_periodStartMillis;
    std::array < float, UPDATES_PER_SECOND * TEMPERATUR_DIFFERENCE_OVER_SEC + 1 > m_tempStore;
public:
    BBQFanOnly(std::shared_ptr<TemperatureSensor> pTempSensor,
               std::shared_ptr<Ventilator> pFan);
    virtual ~BBQFanOnly();
    /**
     * Very important, call this once in 5 seconds
     */
    virtual void handle(const uint32_t millis);
    virtual void setPoint(float temperature);
    virtual float setPoint() const;
    virtual bool lowCharcoal();

    // Fuzzy inputs monitoring
    float tempChangeInput() const {
        return m_tempStore.front() - m_tempStore.back();
    }

    float lastErrorInput() const {
        return m_tempStore.front() - m_setPoint;
    }

    bool ruleFired(uint8_t i);
    void config(const BBQFanOnlyConfig& p_config);
    BBQFanOnlyConfig config() const;

    /**
     * Cretae a fuzzy set from a vector of floats
     * Warning don´t change to &data !!
     */
    template<std::size_t SIZE>
    static FuzzySet* fuzzyFromVector(std::array<float, SIZE>& data, bool flipped) {
        static_assert(SIZE == 2 || SIZE == 4, "Must be 2 or 4");

        if (SIZE == 2) {
            return new FuzzySet(-data[1], -data[0], data[0], data[1]);
        } else if (flipped) {
            return new FuzzySet(-data[3], -data[2], -data[1], -data[0]);
        } else {
            return new FuzzySet(data[0], data[1], data[2], data[3]);
        }
    }

    void init();
    virtual const char* name() const {
        return "fuzzy";
    }

private:
    FuzzyRule* joinSingle(int rule, FuzzySet* fi, FuzzySet* fo);
    FuzzyRule* joinSingleAND(int rule, FuzzySet* fi1, FuzzySet* fi2, FuzzySet* fo);

};
