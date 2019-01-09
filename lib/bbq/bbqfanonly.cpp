#include "bbqfanonly.h"

#include <memory>
#include <algorithm>

#ifdef UNIT_TEST
#include <iostream>
#include <iomanip>
#endif
#include <FuzzyRule.h>
#include <FuzzyComposition.h>
#include <Fuzzy.h>
#include <FuzzyRuleConsequent.h>
#include <FuzzyOutput.h>
#include <FuzzyInput.h>
#include <FuzzyIO.h>
#include <FuzzySet.h>
#include <FuzzyRuleAntecedent.h>

#include <cmath>

#define TEMP_ERROR_INPUT 1
#define TEMP_DELTA_ERROR_INPUT 2
#define TEMP_FILTERED_INPUT 3
#define FAN_INPUT 4
#define FAN_OUTPUT 1
#define LID_ALERT_OUTPUT 2
#define CHARCOAL_ALERT_OUTPUT 3
#define LID_ALERT_RULE 20
#define CHARCOAL_ALERT_RULE 10
#define TEMP_BUFFER_SIZE 100

BBQFanOnly::BBQFanOnly(std::shared_ptr<TemperatureSensor> pTempSensor,
                       std::shared_ptr<Ventilator> pFan) :
    m_tempSensor(pTempSensor),
    m_fan(pFan),
    m_fuzzy(new Fuzzy()),
    m_setPoint(m_tempSensor->get()),
    m_tempLastError(0.0),
    m_fanCurrentSpeed(m_fan->speed()),
    m_tempFiltered(m_tempSensor->get()),
    m_deltaError(0.0),
    m_tempDropFiltered(0.0) {
}

BBQFanOnly::~BBQFanOnly() {
}

void BBQFanOnly::config(const BBQFanOnlyConfig& p_config) {
    m_config = p_config;
}

BBQFanOnlyConfig BBQFanOnly::config() const {
    return m_config;
}

void BBQFanOnly::init() {
    delete m_fuzzy;
    m_fuzzy = new Fuzzy();
    // Create input for Temperature errors
    FuzzyInput* tempErrorInput = new FuzzyInput(TEMP_ERROR_INPUT);
    m_fuzzy->addFuzzyInput(tempErrorInput);

    FuzzySet* tempErrorNegativeHigh = fuzzyFromVector(m_config.temp_error_hight, true);
    tempErrorInput->addFuzzySet(tempErrorNegativeHigh);
    FuzzySet* tempErrorNegativeMedium = fuzzyFromVector(m_config.temp_error_medium, true);
    tempErrorInput->addFuzzySet(tempErrorNegativeMedium);
    FuzzySet* tempErrorLow = fuzzyFromVector(m_config.temp_error_low, false);
    tempErrorInput->addFuzzySet(tempErrorLow);
    FuzzySet* tempErrorPositiveMedium = fuzzyFromVector(m_config.temp_error_medium, false);
    tempErrorInput->addFuzzySet(tempErrorPositiveMedium);
    FuzzySet* tempErrorPositiveHigh = fuzzyFromVector(m_config.temp_error_hight, false);
    tempErrorInput->addFuzzySet(tempErrorPositiveHigh);


    // Create Output for Fan
    FuzzyOutput* fan = new FuzzyOutput(FAN_OUTPUT);
    m_fuzzy->addFuzzyOutput(fan);

    FuzzySet* fanLow = fuzzyFromVector(m_config.fan_low, false);
    fan->addFuzzySet(fanLow);
    FuzzySet* fanMedium = fuzzyFromVector(m_config.fan_medium, false);
    fan->addFuzzySet(fanMedium);
    FuzzySet* fanHigh = fuzzyFromVector(m_config.fan_high, false);
    fan->addFuzzySet(fanHigh);


    // Fuzzy inputs for temperature drops
    FuzzyInput* tempChance = new FuzzyInput(TEMP_DELTA_ERROR_INPUT);
    m_fuzzy->addFuzzyInput(tempChance);

    FuzzySet* tempChangeDropFast = fuzzyFromVector(m_config.temp_change_fast, true);
    tempChance->addFuzzySet(tempChangeDropFast);

    // Temperature control
    uint8_t rule = 30;

    joinSingle(rule++, tempErrorNegativeHigh, fanHigh);
    joinSingle(rule++, tempErrorNegativeMedium, fanHigh);
    joinSingle(rule++, tempErrorLow, fanMedium);
    joinSingle(rule++, tempErrorPositiveMedium, fanLow);
    joinSingle(rule++, tempErrorPositiveHigh, fanLow);

    // Lid alert rules
    FuzzyInput* tmpFilteredInput = new FuzzyInput(TEMP_FILTERED_INPUT);
    m_fuzzy->addFuzzyInput(tmpFilteredInput);
    FuzzySet* tempLidDropFast = fuzzyFromVector(m_config.temp_change_fast, true);
    tmpFilteredInput->addFuzzySet(tempLidDropFast);
    FuzzyOutput* lidAlertOutput = new FuzzyOutput(LID_ALERT_OUTPUT);
    m_fuzzy->addFuzzyOutput(lidAlertOutput);
    FuzzySet* lidAlertSet = new FuzzySet(-100, 0, 0, 100);
    lidAlertOutput->addFuzzySet(lidAlertSet);
    joinSingle(LID_ALERT_RULE, tempLidDropFast, lidAlertSet);

    // CHarcoal rules
    // FuzzyInput* fanInput = new FuzzyInput(FAN_INPUT);
    // m_fuzzy->addFuzzyInput(fanInput);
    // FuzzyOutput* charcoalAlertOutput = new FuzzyOutput(CHARCOAL_ALERT_OUTPUT);
    // m_fuzzy->addFuzzyOutput(charcoalAlertOutput);
    // FuzzySet* lowCharcoaltempDropSlow = new FuzzySet(-2 / divider, -1 / divider, -1 / divider, 0 / divider);
    // tempChance->addFuzzySet(lowCharcoaltempDropSlow);
    // FuzzySet* lowCharcoalFanMedium = new FuzzySet(20, 40, 40, 60);
    // charcoalAlertOutput->addFuzzySet(lowCharcoalFanMedium);
    // FuzzySet* charcoalAlertSet = new FuzzySet(-100, 0, 0, 100);
    // charcoalAlertOutput->addFuzzySet(charcoalAlertSet);
    // joinSingleAND(10, lowCharcoalFanMedium, lowCharcoaltempDropSlow, charcoalAlertSet);
}


void BBQFanOnly::setPoint(float setTemp) {
    m_setPoint = setTemp;
}
float BBQFanOnly::setPoint() const {
    return m_setPoint;
}

void BBQFanOnly::handle() {
    m_deltaError = ((m_tempSensor->get() - m_setPoint) - m_tempLastError) * 1000.0;
    m_tempLastError = m_tempSensor->get() - m_setPoint;
    m_tempDropFiltered = m_tempSensor->get() - m_tempFiltered;

    // Filtered temperature is to understand if the temperature suddenly drops over a larger time T
    // Usefull for LID open and low charcoal alerts
    m_tempFiltered = m_tempFiltered + (m_tempSensor->get() - m_tempFiltered) * m_config.temp_alpha;

    m_fuzzy->setInput(TEMP_FILTERED_INPUT, m_tempDropFiltered);

    // Feed delta error (t-1)
    m_fuzzy->setInput(TEMP_DELTA_ERROR_INPUT, m_deltaError);
    // Feed temp error
    m_fuzzy->setInput(TEMP_ERROR_INPUT, m_tempLastError);
    // Fan input
    m_fuzzy->setInput(FAN_INPUT, m_fanCurrentSpeed);
    // Run fuzzy rules
    m_fuzzy->fuzzify();

    // This provides a small filter to ensure that our fan doesnt jump to much
    m_fanCurrentSpeed = m_fanCurrentSpeed + ( m_fuzzy->defuzzify(FAN_OUTPUT) - m_fanCurrentSpeed) * 0.25f;

    // Finally set fan
    m_fan->speed(m_fanCurrentSpeed);

#ifdef UNIT_TEST
    std::cout << " f3:" << m_fuzzy->defuzzify(3) << " td:" << std::setw(6) << m_lastTempError << " ch:" << std::setw(6) << tempDiff * 1000;

    for (int i = 0; i < 20; i++) {
        std::cout << std::fixed << std::setprecision(3) << m_fuzzy->isFiredRule(i + 29);

        if (i % 5 == 0) {
            std::cout << " ";
        }
    };

#endif
}

float BBQFanOnly::deltaErrorInput() const {
    return m_deltaError;
}
float BBQFanOnly::tempDropFilteredInput() const {
    return m_tempDropFiltered;
}
float BBQFanOnly::lastErrorInput() const {
    return m_tempLastError;
}

bool BBQFanOnly::lowCharcoal() {
    return m_fuzzy->isFiredRule(CHARCOAL_ALERT_RULE);
}

bool BBQFanOnly::lidOpen() {
    return m_fuzzy->isFiredRule(LID_ALERT_RULE);
}

FuzzyRule* BBQFanOnly::joinSingle(int rule, FuzzySet* fi, FuzzySet* fo) {
    FuzzyRuleAntecedent* ifCondition = new FuzzyRuleAntecedent();
    ifCondition->joinSingle(fi);
    FuzzyRuleConsequent* thenConsequence = new FuzzyRuleConsequent();
    thenConsequence->addOutput(fo);
    FuzzyRule* fr = new FuzzyRule(rule, ifCondition, thenConsequence);
    m_fuzzy->addFuzzyRule(fr);
    return fr;
}

FuzzyRule* BBQFanOnly::joinSingleAND(int rule, FuzzySet* fi1, FuzzySet* fi2, FuzzySet* fo) {
    FuzzyRuleAntecedent* ifCondition = new FuzzyRuleAntecedent();
    ifCondition->joinWithAND(fi1, fi2);
    FuzzyRuleConsequent* thenConsequence = new FuzzyRuleConsequent();
    thenConsequence->addOutput(fo);
    FuzzyRule* fr = new FuzzyRule(rule, ifCondition, thenConsequence);
    m_fuzzy->addFuzzyRule(fr);
    return fr;
}
