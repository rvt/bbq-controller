
#include "displaycontroller.h"
#include <icons.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include "icons.h"
#include <digitalknob.h>
#include <memory>
#include <math.h>
#include "settingsdto.h"
#include <ventilator.h>

/* Technical debt, all these externals should be passed to the display object somehow */
extern std::unique_ptr<BBQFanOnly> bbqController;
extern std::shared_ptr<TemperatureSensor> temperatureSensor1;
extern std::shared_ptr<TemperatureSensor> temperatureSensor2;
extern std::shared_ptr<Ventilator> ventilator1;
extern PubSubClient mqttClient;
extern OLEDDisplayUi ui;
extern DigitalKnob digitalKnob;
extern std::shared_ptr<AnalogIn> analogIn;
extern SettingsDTO settingsDTO;
extern std::shared_ptr<Ventilator> ventilator1;


// Temporary untill we can have the display functions handle object variables
static std::unique_ptr<NumericKnob> m_temperatureSetPointKnob;
static std::unique_ptr<NumericKnob> m_fanOverrideKnob;
static std::unique_ptr<NumericKnob> m_menuKnob;

#define MENU_BLOCK_SIZE 6
#define MENU_FONT_NAME ArialMT_Plain_10
#define MENU_FONT_SIZE 10

void DisplayController::init() {

    m_temperatureSetPointKnob.reset(new NumericKnob(analogIn, bbqController->setPoint(), 90, 240, 0.1));
    m_fanOverrideKnob.reset(new NumericKnob(analogIn, ventilator1->speedOverride(), -1, 100, 0.1));
    m_menuKnob.reset(new NumericKnob(analogIn, 0, 0, 2, 0.01));

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

    menuScreens = {
        menuMain,
        menuSetDesiredTemperature,
        menuOverrideFan
    };

    STATE_STARTSCREEN = new State([&]() {
        ui.setOverlays(displayOverlay.data(), displayOverlay.size());
        ui.setFrames(startScreens.data(), startScreens.size());
        return STATE_WAITLOGO;
    });

    STATE_WAITLOGO = new StateTimed((2500), [&]() {
        // Display splash screen
        return STATE_CHANGETORUNSCREEN;
    });

    STATE_CHANGETORUNSCREEN = new State([&]() {
        ui.setOverlays(displayOverlay.data(), displayOverlay.size());
        ui.setFrames(normalRunScreens.data(), normalRunScreens.size());
        ui.enableAllIndicators();
        ui.enableAutoTransition();
        return STATE_RUNSCREEN;
    });

    STATE_RUNSCREEN = new State([&]() {
        if (digitalKnob.isEdgeUp()) {
            return STATE_CHANGETOMENUBUTTONRELEASE;
        }

        return STATE_RUNSCREEN;
    });

    STATE_CHANGETOMENUBUTTONRELEASE = new State([&]() {
        if (digitalKnob.current() == false) {
            // Clears the internal status so we don´t get a false click later
            digitalKnob.reset();
            return STATE_CHANGETOMENUSCREEN;
        }

        return STATE_CHANGETOMENUBUTTONRELEASE;
    });

    STATE_CHANGETOMENUSCREEN = new State([&]() {
        ui.setFrames(menuScreens.data(), menuScreens.size());
        ui.switchToFrame(0);
        ui.disableAutoTransition();
        m_menuKnob->value(0);
        return STATE_SELECTMENUITEM;
    });

    STATE_SELECTMENUITEM = new State([&]() {
        uint8_t menu = ((int)round(m_menuKnob->value()));

        if (digitalKnob.isSingle()) {
            ui.switchToFrame(menu);

            switch (menu) {
                case 0 :
                    return STATE_CHANGETORUNSCREEN;

                case 1 :
                    m_temperatureSetPointKnob->value(bbqController->setPoint());
                    return STATE_SETTEMP;

                case 2 :
                    m_fanOverrideKnob->value(ventilator1->speedOverride());
                    return STATE_SETFAN;
            };
        }

        m_menuKnob->handle();
        return STATE_SELECTMENUITEM;
    });

    STATE_SETTEMP = new State([&]() {
        if (digitalKnob.isSingle()) {
            float value = round(m_temperatureSetPointKnob->value() * 2.0f) / 2.0f;
            settingsDTO.data()->setPoint = value;
            bbqController->setPoint(value);
            return STATE_CHANGETOMENUSCREEN;
        }

        if (!digitalKnob.isLong()) {
            m_temperatureSetPointKnob->handle();
        }

        return STATE_SETTEMP;
    });

    STATE_SETFAN = new State([&]() {
        if (digitalKnob.isSingle()) {
            float value = round(m_fanOverrideKnob->value());
            ventilator1->speedOverride(value);
            return STATE_CHANGETOMENUSCREEN;
        }

        if (!digitalKnob.isLong()) {
            m_fanOverrideKnob->handle();
        }

        return STATE_SETFAN;
    });

    menuSequence.reset(new StateMachine({
        STATE_STARTSCREEN,
        STATE_WAITLOGO,
        STATE_CHANGETORUNSCREEN,
        STATE_RUNSCREEN,
        STATE_CHANGETOMENUSCREEN,
        STATE_CHANGETOMENUBUTTONRELEASE,
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
    char buffer[16];
    display->setFont(ArialMT_Plain_24);
    display->setTextAlignment(TEXT_ALIGN_LEFT);
    sprintf(buffer, "%.1f°C", bbqController->setPoint());
    display->drawString(x + 0 + knob_width + 4, y + 20, buffer);
    display->drawXbm(x, y + 20, knob_width, knob_height, knob_bits);
}

void DisplayController::currentTemperatureSensor1(OLEDDisplay* display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
    char buffer[16];
    display->setFont(ArialMT_Plain_24);
    display->setTextAlignment(TEXT_ALIGN_LEFT);
    sprintf(buffer, "1 %.1f°C", temperatureSensor1->get());
    display->drawString(x + 0 + thermometer_width + 4, y + 20, buffer);
    display->drawXbm(x, y + 20, thermometer_width, thermometer_height, (uint8_t*)thermometer_bits);
}

void DisplayController::currentTemperatureSensor2(OLEDDisplay* display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
    char buffer[16];
    display->setFont(ArialMT_Plain_24);
    display->setTextAlignment(TEXT_ALIGN_LEFT);
    sprintf(buffer, "2 %.1f°C", temperatureSensor2->get());
    display->drawString(x + 0 + thermometer_width + 4, y + 20, buffer);
    display->drawXbm(x, y + 20, thermometer_width, thermometer_height, (uint8_t*)thermometer_bits);
}

void DisplayController::currentFanSpeed(OLEDDisplay* display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
    display->setFont(ArialMT_Plain_10);
    display->setTextAlignment(TEXT_ALIGN_RIGHT);
    float value = ventilator1->speedOverride();

    if (!ventilator1->isOverride()) {
        display->drawString(x + 128, y + 20, "Auto");
    } else {
        display->drawString(x + 128, y + 20 + 10, "Manual");
    }

    char buffer[16];
    display->setFont(ArialMT_Plain_24);
    display->setTextAlignment(TEXT_ALIGN_LEFT);
    value = ventilator1->speed();

    if (value < 0.1) {
        sprintf(buffer, "Off");
    } else {
        sprintf(buffer, "%3.0f%%", ventilator1->speed());
    }

    display->drawString(x + 0 + fan_width + 4, y + 20, buffer);
    display->drawXbm(x, y + 20, fan_width, fan_height, fan_bits);
}

void DisplayController::startScreen(OLEDDisplay* display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
    display->drawXbm(x, y, logo_width, logo_height, logo_bits);
}

void DisplayController::menuMain(OLEDDisplay* display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
    display->setFont(MENU_FONT_NAME);
    display->setTextAlignment(TEXT_ALIGN_LEFT);

    uint8_t menu = ((int)round(m_menuKnob->value()));
    int8_t markerPos = -6;

    switch (menu) {
        case 0:
            markerPos = 12;
            break;

        case 1:
            markerPos = 24;
            break;

        case 2:
            markerPos = 36;
            break;
    }

    display->fillRect(x + 0,   y + (MENU_FONT_SIZE / 2 - (MENU_BLOCK_SIZE / 2)) - 1 + markerPos + (MENU_BLOCK_SIZE / 2), MENU_BLOCK_SIZE, MENU_BLOCK_SIZE);
    display->drawString(x + 8, y + 12, "Back");
    display->drawString(x + 8, y + 24, "Set Temperature");
    display->drawString(x + 8, y + 36, "Override fan");
}

void DisplayController::menuSetDesiredTemperature(OLEDDisplay* display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
    display->setFont(ArialMT_Plain_16);
    display->setTextAlignment(TEXT_ALIGN_LEFT);
    float value = round(m_temperatureSetPointKnob->value() * 2.0f) / 2.0f;
    char buffer[16];
    sprintf(buffer, "Set: %.1f°C", value);
    display->drawString(x + 20, y + 22, buffer);
    display->drawXbm(x, y + 20, thermometer_width, thermometer_height, (uint8_t*)thermometer_bits);
}

void DisplayController::menuOverrideFan(OLEDDisplay* display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
    display->setFont(ArialMT_Plain_16);
    display->setTextAlignment(TEXT_ALIGN_LEFT);
    int16_t value = round(m_fanOverrideKnob->value());
    char buffer[16];

    if (value == -1) {
        sprintf(buffer, "Set: Auto");
    } else if (value == 0) {
        sprintf(buffer, "Set: Off");
    } else {
        sprintf(buffer, "Set: %i%%", value);
    }

    display->drawString(x + 40, y + 22, buffer);
    display->drawXbm(x, y + 20, fan_width, fan_height, fan_bits);
}

void DisplayController::normalOverlayDisplay(OLEDDisplay* display, OLEDDisplayUiState* state) {
    char buffer[16];
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