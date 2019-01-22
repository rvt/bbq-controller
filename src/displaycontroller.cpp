
#include "displaycontroller.h"
#include <icons.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include "icons.h"
#include <digitalknob.h>
#include <memory>

/* Technical debt, we should get the display values from some way?? */
extern std::unique_ptr<BBQFanOnly> bbqController;
extern std::shared_ptr<TemperatureSensor> temperatureSensor1;
extern std::shared_ptr<TemperatureSensor> temperatureSensor2;
extern std::shared_ptr<Ventilator> ventilator1;
extern PubSubClient mqttClient;
extern OLEDDisplayUi ui;
extern DigitalKnob digitalKnob;


void DisplayController::init() {

    startScreens = {
        startScreen
    };

    normalRunScreens = {
        currentTemperatureSetting,
        currentTemperatureSensor1,
        currentTemperatureSensor2,
        currentFanSpeed
    };

    displayOverlay = {
        normalOverlayDisplay
    };

    STATE_STARTSCREEN = new State([&]() {
        ui.setOverlays(displayOverlay.data(), displayOverlay.size());
        ui.setFrames(startScreens.data(), startScreens.size());
        return STATE_WAITLOGO;
    });

    STATE_WAITLOGO = new StateTimed((2000), [&]() {
        return STATE_CHANGETORUNSCREEN;
    });

    STATE_CHANGETORUNSCREEN = new State([&]() {
        ui.setOverlays(displayOverlay.data(), displayOverlay.size());
        ui.setFrames(normalRunScreens.data(), normalRunScreens.size());
        return STATE_RUNSCREEN;
    });

    STATE_RUNSCREEN = new State([&]() {
        if (digitalKnob.current()) {
            return STATE_CHANGETOMENUSCREEN;
        }

        return STATE_RUNSCREEN;
    });

    STATE_CHANGETOMENUSCREEN = new State([&]() {
        ui.setFrames(normalRunScreens.data(), normalRunScreens.size());
        return STATE_SELECTMENUITEM;
    });

    STATE_SELECTMENUITEM = new State([&]() {
        if (!digitalKnob.current()) {
            return STATE_CHANGETORUNSCREEN;
        }

        return STATE_SELECTMENUITEM;
    });

    STATE_SETTEMP = new State([&]() {
        return STATE_SETTEMP;
    });

    STATE_SETFAN = new State([&]() {
        return STATE_SETFAN;
    });

    menuSequence.reset(new StateMachine({
        STATE_STARTSCREEN,
        STATE_WAITLOGO,
        STATE_CHANGETORUNSCREEN,
        STATE_RUNSCREEN,
        STATE_CHANGETOMENUSCREEN,
        STATE_SELECTMENUITEM,
        STATE_SETTEMP,
        STATE_SETFAN
    }));

    menuSequence->start();

}

uint32_t DisplayController::handle() {
    menuSequence->handle();
    return 0;
}

void DisplayController::currentTemperatureSetting(OLEDDisplay* display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
    char buffer[32];
    // Set Temperature
    display->setFont(ArialMT_Plain_24);
    display->setTextAlignment(TEXT_ALIGN_LEFT);
    sprintf(buffer, "%.1f°C", bbqController->setPoint());
    display->drawString(x + 0 + knob_width + 4, y + 20, buffer);
    display->drawXbm(x, y + 20, knob_width, knob_height, knob_bits);
}

void DisplayController::currentTemperatureSensor1(OLEDDisplay* display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
    char buffer[32];
    // Current temperature speed
    display->setFont(ArialMT_Plain_24);
    display->setTextAlignment(TEXT_ALIGN_LEFT);
    sprintf(buffer, "1 %.1f°C", 1.0f/*temperatureSensor1->get()*/);
    display->drawString(x + 0 + thermometer_width + 4, y + 20, buffer);
    display->drawXbm(x, y + 20, thermometer_width, thermometer_height, (uint8_t*)thermometer_bits);
}

void DisplayController::currentTemperatureSensor2(OLEDDisplay* display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
    char buffer[32];
    // Current temperature speed
    display->setFont(ArialMT_Plain_24);
    display->setTextAlignment(TEXT_ALIGN_LEFT);
    sprintf(buffer, "2 %.1f°C", temperatureSensor2->get());
    display->drawString(x + 0 + thermometer_width + 4, y + 20, buffer);
    display->drawXbm(x, y + 20, thermometer_width, thermometer_height, (uint8_t*)thermometer_bits);
}

void DisplayController::currentFanSpeed(OLEDDisplay* display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
    char buffer[32];
    // Ventilator speed
    display->setFont(ArialMT_Plain_24);
    display->setTextAlignment(TEXT_ALIGN_LEFT);
    sprintf(buffer, "%3.0f%%", ventilator1->speed());
    display->drawString(x + 0 + fan_width + 4, y + 20, buffer);
    display->drawXbm(x, y + 20, fan_width, fan_height, fan_bits);
}

void DisplayController::startScreen(OLEDDisplay* display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
    display->drawXbm(x, y, logo_width, logo_height, logo_bits);
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