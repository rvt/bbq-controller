#pragma once

#include <memory>
#include <array>
#include <cstddef>

#include <OLEDDisplay.h>
#include <OLEDDisplayUi.h>
#include <bbqfanonly.h>
#include <statemachine.h>


class DisplayController {
private:
    int foo;
public:
    void init();

private:
    ///////////////////////////////////////////////////////////////////////////
    //  UI Rendering
    ///////////////////////////////////////////////////////////////////////////

    static void currentTemperatureSetting(OLEDDisplay* display, OLEDDisplayUiState* state, int16_t x, int16_t y);
    static void currentTemperatureSensor1(OLEDDisplay* display, OLEDDisplayUiState* state, int16_t x, int16_t y);
    static void currentTemperatureSensor2(OLEDDisplay* display, OLEDDisplayUiState* state, int16_t x, int16_t y);
    static void currentFanSpeed(OLEDDisplay* display, OLEDDisplayUiState* state, int16_t x, int16_t y);
    static void normalOverlayDisplay(OLEDDisplay* display, OLEDDisplayUiState* state);

    // All screens for normal operation
    std::array<FrameCallback, 4> normalRunScreens = { };
    std::array<OverlayCallback, 1> displayOverlay = { };
    std::array<FrameCallback, 3> menuScreens = { };


    std::unique_ptr<StateMachine> bootSequence;

};
