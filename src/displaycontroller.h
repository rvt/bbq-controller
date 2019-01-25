#pragma once

#include <memory>
#include <array>
#include <cstddef>
#include <algorithm>

#include <OLEDDisplay.h>
#include <OLEDDisplayUi.h>
#include <bbqfanonly.h>
#include <statemachine.h>
#include <digitalinput.h>
#include <numericknob.h>


class DisplayController {
private:
public:
    void init();
    uint32_t handle();

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


    State* STATE_STARTSCREEN;
    State* STATE_WAITLOGO;
    State* STATE_CHANGETORUNSCREEN;
    State* STATE_RUNSCREEN;
    State* STATE_CHANGETOMENUSCREEN;
    State* STATE_SELECTMENUITEM;
    State* STATE_SETTEMP;
    State* STATE_SETFAN;
    State* STATE_CHANGETOMENUBUTTONRELEASE;
    std::unique_ptr<StateMachine> menuSequence;


};
