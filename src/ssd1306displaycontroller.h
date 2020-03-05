#pragma once

#include <memory>
#include <array>
#include <cstddef>
#include <algorithm>

#include <bbqfanonly.h>
#include <statemachine.h>
#include <digitalinput.h>
#include <numericknob.h>
#include <OLEDDisplayUi.h>

class SSD1306Brzo;
class OLEDDisplay;
class OLEDDisplayUiState;

class SSD1306DisplayController {
public:
    void init();
    uint32_t handle();
    SSD1306DisplayController(uint8_t m_wireSda,  uint8_t m_wireScl);
    virtual ~SSD1306DisplayController();

private:
    ///////////////////////////////////////////////////////////////////////////
    //  UI Rendering
    ///////////////////////////////////////////////////////////////////////////
    static void startScreen(OLEDDisplay* display, OLEDDisplayUiState* state, int16_t x, int16_t y);

    static void currentTemperatureSetting(OLEDDisplay* display, OLEDDisplayUiState* state, int16_t x, int16_t y);
    static void currentTemperatureSensor1(OLEDDisplay* display, OLEDDisplayUiState* state, int16_t x, int16_t y);
    static void currentTemperatureSensor2(OLEDDisplay* display, OLEDDisplayUiState* state, int16_t x, int16_t y);

    static void menuMain(OLEDDisplay* display, OLEDDisplayUiState* state, int16_t x, int16_t y);
    static void menuSetDesiredTemperature(OLEDDisplay* display, OLEDDisplayUiState* state, int16_t x, int16_t y);
    static void menuOverrideFan(OLEDDisplay* display, OLEDDisplayUiState* state, int16_t x, int16_t y);

    static void currentFanSpeed(OLEDDisplay* display, OLEDDisplayUiState* state, int16_t x, int16_t y);
    static void normalOverlayDisplay(OLEDDisplay* display, OLEDDisplayUiState* state);

    // All screens for normal operation
    std::array<FrameCallback, 1> startScreens = { };
    std::array<FrameCallback, 4> normalRunScreens = { };
    std::array<OverlayCallback, 1> displayOverlay = { };
    std::array<FrameCallback, 3> menuScreens = { };

    std::unique_ptr<StateMachine> menuSequence;

    SSD1306Brzo* display;
    OLEDDisplayUi* ui;
    uint32_t m_lastMillis;;


};
