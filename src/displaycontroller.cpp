
#include "displaycontroller.h"
#include <icons.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <memory>

/* Technical debt, we should get the display values from some way?? */
extern std::unique_ptr<BBQFanOnly> bbqController;
extern std::shared_ptr<TemperatureSensor> temperatureSensor1;
extern std::shared_ptr<TemperatureSensor> temperatureSensor2;
extern std::shared_ptr<Ventilator> ventilator1;
extern PubSubClient mqttClient;


void DisplayController::init() {

    normalRunScreens = {
        currentTemperatureSetting,
        currentTemperatureSensor1,
        currentTemperatureSensor2,
        currentFanSpeed
    };

    displayOverlay = {
        normalOverlayDisplay
    };

    State* STATE_MENU;
    State* STATE_SETTEMP;
    State* STATE_SETFAN;

    STATE_MENU = new State([&STATE_MENU, &STATE_SETTEMP]() {
        return STATE_MENU;
    });
    STATE_SETTEMP = new State([&STATE_SETTEMP, &STATE_SETFAN]() {
        return STATE_SETTEMP;
    });
    STATE_SETFAN = new State([&STATE_SETFAN, &STATE_MENU]() {
        return STATE_SETFAN;
    });

    bootSequence.reset(new StateMachine({
        STATE_MENU,
        STATE_SETTEMP,
        STATE_SETFAN
    }));

}

void DisplayController::currentTemperatureSetting(OLEDDisplay* display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
    char buffer[32];
    // Set Temperature
    display->setFont(ArialMT_Plain_24);
    display->setTextAlignment(TEXT_ALIGN_LEFT);
    sprintf(buffer, "%.1f°C", bbqController->setPoint());
    display->drawString(x + 0 + knob_width + 4, y + 20, buffer);
    display->drawXbm(0, y + 20, knob_width, knob_height, knob_bits);
}

void DisplayController::currentTemperatureSensor1(OLEDDisplay* display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
    char buffer[32];
    // Current temperature speed
    display->setFont(ArialMT_Plain_24);
    display->setTextAlignment(TEXT_ALIGN_LEFT);
    sprintf(buffer, "1 %.1f°C", 1.0f/*temperatureSensor1->get()*/);
    display->drawString(x + 0 + thermometer_width + 4, y + 20, buffer);
    display->drawXbm(0, y + 20, thermometer_width, thermometer_height, (uint8_t*)thermometer_bits);
}

void DisplayController::currentTemperatureSensor2(OLEDDisplay* display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
    char buffer[32];
    // Current temperature speed
    display->setFont(ArialMT_Plain_24);
    display->setTextAlignment(TEXT_ALIGN_LEFT);
    sprintf(buffer, "2 %.1f°C", temperatureSensor2->get());
    display->drawString(x + 0 + thermometer_width + 4, y + 20, buffer);
    display->drawXbm(0, y + 20, thermometer_width, thermometer_height, (uint8_t*)thermometer_bits);
}

void DisplayController::currentFanSpeed(OLEDDisplay* display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
    char buffer[32];
    // Ventilator speed
    display->setFont(ArialMT_Plain_24);
    display->setTextAlignment(TEXT_ALIGN_LEFT);
    sprintf(buffer, "%3.0f%%", ventilator1->speed());
    display->drawString(x + 0 + fan_width + 4, y + 20, buffer);
    display->drawXbm(0, y + 20, fan_width, fan_height, fan_bits);
}

void DisplayController::normalOverlayDisplay(OLEDDisplay* display, OLEDDisplayUiState* state) {
    char buffer[32];
    // Current temperature speed
    display->setFont(ArialMT_Plain_10);
    display->setTextAlignment(TEXT_ALIGN_RIGHT);

    sprintf(buffer, "%.1f°C", temperatureSensor1->get());
    display->drawString(128, 0, buffer);

    if (WiFi.status() == WL_CONNECTED) {
        display->drawXbm(0, 0, wifiicon10x10_width, wifiicon10x10_height, wifi10x10_png_bits);
    }

    if (mqttClient.connected())  {
        display->drawXbm(wifiicon10x10_width + 4, 0, mqttcloud_width, mqttcloud_height, mqttcloud_bits);
    }
}