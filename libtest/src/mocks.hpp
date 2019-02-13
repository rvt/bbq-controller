#pragma once

#include <temperaturesensor.h>
#include <ventilator.h>
#include <demo.h>


/** very simple oven simulation
 * airflow == temperature
 */

#define  NUMBERS_SIZE 10000

class MockedOven  {
private:
    float mAirFlow;
    float mTemperature;
    float mBaseTemp;

    float m_cAirFlow;
    bool mUp;

    float numbers[NUMBERS_SIZE] = {};
    int numCalled;
    bool m_lidOpen;

public:
    MockedOven() : mAirFlow(0.0f), mTemperature(20.0f), mBaseTemp(20.0f), m_cAirFlow(0.0f), mUp(true), numCalled(0), m_lidOpen(false) {
        for (int i = 0; i < NUMBERS_SIZE; i++) {
            numbers[i] = 0.0f;
        }
    }

    void handle() {
        float alpha;

        if (mUp) {
            alpha = 0.02f;
        } else {
            alpha = 0.01f;
        }

        float probe = mAirFlow;

        if (m_lidOpen) {
            probe = 0;
            alpha = 0.1f;
        }

        for (int i = 0; i < (m_lidOpen ? 5 : 1); i++) {
            m_cAirFlow = m_cAirFlow + (probe - m_cAirFlow) * alpha;
            memmove(&numbers[1], &numbers[0], sizeof(float) * (NUMBERS_SIZE - 1));
            numbers[0] = m_cAirFlow + rand() % 16 - 8 + 2;
        }

        float sum = 0.0f;

        for (int i = 0; i < NUMBERS_SIZE; i++) {
            sum += numbers[i];
        }

        mTemperature = (sum / NUMBERS_SIZE) * 2 + mBaseTemp;
    }

    void lidOpen(bool lo) {
        m_lidOpen = true;
    }

    void airFlow(float flow) {
        flow = flow * 1;
        mUp = flow > mAirFlow;
        mAirFlow = flow;
    }

    float temperature() {
        return mTemperature;
    }
};

