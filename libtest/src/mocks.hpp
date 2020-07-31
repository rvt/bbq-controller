#pragma once

#include <temperaturesensor.h>
#include <ventilator.h>
#include <demo.h>
#include <math.h>
#include <algorithm>


/** very simple oven simulation
 * airflow == temperature
 */

#define  NUMBERS_SIZE 10000


class Oven {
    virtual void handle(const uint32_t millis) = 0;
    virtual void lidOpen(bool lo) = 0;
    virtual void airFlow(float flow) = 0;
    virtual float temperature() = 0;
};

class briquette {
    float s_maxBurnTime = 1800.0;
    float m_potentialEnergy = s_maxBurnTime;
    float m_burnRate = 0.0;
    uint32_t m_time = 0;

    float m_lastAirFlow = -1;
    float m_lastAlpha = 0;
    bool m_isBurning = true;
    bool m_up = true;
    float m_alphaAdjust = 1.0f;
public:
    float handle(float airFlow) {
        assert(airFlow >= 0.0f && airFlow <= 100.0f);


        if (!m_isBurning) {
            return 0.0f;
        }

        if (airFlow > m_lastAirFlow) {
            m_up = true;
            m_lastAlpha = 0.004 * m_alphaAdjust;
        } else if (airFlow < m_lastAirFlow) {
            m_up = false;
            m_lastAlpha = 0.002 * m_alphaAdjust;
        }

        m_lastAirFlow = airFlow;

        // maps airflow from 0..1
        float af = fmap(airFlow, 0.0f, 100.0f, 0.0f, 1.0f);

        // maps burn potential from 0..2 so we can put it in
        // https://www.wolframalpha.com/input/?i=plot+-%28x-1%29%5E8%2B1+from+-0.1+to+2
        // currentBurn is 0..1
        float burnPotential = fmap(m_potentialEnergy, 0.0f, s_maxBurnTime, 0.1f, 1.9f);
        float currentBurn = (-pow(burnPotential - 1, 4) + 1.0f) * af;

        if (m_up) {
            m_burnRate = (m_burnRate + (currentBurn - m_burnRate) * m_lastAlpha) * 1.0f;
        } else {
            m_burnRate = (m_burnRate + (0.0f - m_burnRate) * m_lastAlpha) * 1.0f;
        }

        // https://www.wolframalpha.com/input/?i=plot+1%2F%281+%2B+2%5E%28-x%29%29+from+-10+to10

        // Reduce energy
        if (m_burnRate >= 1.0f) {
            m_burnRate = 1.0f;
        }

        if (m_burnRate < 0) {
            m_burnRate = 0.0f;
        }

        m_potentialEnergy = m_potentialEnergy - m_burnRate;

        if (m_potentialEnergy < 0) {
            m_potentialEnergy = 0;
        }

        return m_burnRate;
    }


    void startBurn() {
        m_isBurning = true;
    }

    void alphaAdjust(float aa) {
        m_alphaAdjust = aa;
    }

    static float fmap(float value, float in_min, float in_max, float out_min, float out_max) {
        return (value - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
    }

};


class MockedOven  : public Oven {
private:
    std::vector<briquette> briqettes;
    float m_AirFlow = 0.0;
    float m_temperature;
    bool m_lidOpen;
    int m_totalBriqettes = 8;
    int m_maxBriqettes = 10;
    uint32_t lastTIme = 0;
public:
    MockedOven() {
        briqettes.resize(m_totalBriqettes);
    }

    void handle(const uint32_t millis) {
        if (millis - lastTIme > 900 * 1000) {
            lastTIme = millis;
            m_totalBriqettes = m_totalBriqettes + 1;

            if (m_totalBriqettes >= m_maxBriqettes) {
                m_totalBriqettes = m_maxBriqettes;
            } else {
                briqettes.resize(m_totalBriqettes);
            }
        }

        m_temperature = 0.0;

        for (auto& value : briqettes) {
            float flow = (rand() % 20) - 10.f + m_AirFlow;

            if (flow < 0) {
                flow = 0;
            }

            if (flow > 100) {
                flow = 100;
            }

            m_temperature += value.handle(flow) * 1.8f;
        }

        // Temperure is now 0..m_totalBriqettes

        // https://www.wolframalpha.com/input/?i=plot+1%2F%281+%2B+2%5E%28-x%29%29+from+-10+to10
        float bMapped = briquette::fmap(m_totalBriqettes, 0.0f, m_maxBriqettes, -10.0f, 10.0f);
        float maxTempTotalOvenCoef = 1.0f / (1.0f + pow(2.0f, -bMapped));

        m_temperature = (250.0f / m_maxBriqettes) * m_temperature * maxTempTotalOvenCoef;
    }

    void lidOpen(bool lo) {
        m_lidOpen = true;
    }

    void airFlow(float flow) {
        m_AirFlow = flow;
    }

    float temperature() {
        return m_temperature;
    }
};

