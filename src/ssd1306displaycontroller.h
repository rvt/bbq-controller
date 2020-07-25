#pragma once

// We can properly do better than this
//#if defined(ESP8266)
#include <memory>
#include <array>
#include <cstddef>
#include <algorithm>

#include <bbqfanonly.h>
#include <statemachine.h>
#include <digitalinput.h>
#include <numericknob.h>
#include <OLEDDisplayUi.h>

#if defined(ESP8266)
#include <Ethernet.h>
#endif

#if defined(ESP32)
    #include <WiFi.h>
    #include <esp_wifi.h>      
#elif defined(ESP8266)
        #include <ESP8266WiFi.h>
#endif

#include "displayController.h"

class SSD1306Brzo;
class OLEDDisplay;
class OLEDDisplayUiState;

class SSD1306DisplayController : public DisplayController {
public:
    SSD1306DisplayController(uint8_t m_wireSda,  uint8_t m_wireScl);
    virtual ~SSD1306DisplayController();
    virtual void init();
    virtual int32_t handle();

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
    std::array<FrameCallback, 3> normalRunScreens = { };
    std::array<OverlayCallback, 1> displayOverlay = { };
    std::array<FrameCallback, 3> menuScreens = { };

    std::unique_ptr<StateMachine> menuSequence;

    SSD1306Brzo* display;
    OLEDDisplayUi* ui;
    uint32_t m_lastMillis;

    void toStringIp(const IPAddress& ip, char* ipAddress) {
        snprintf(ipAddress, 16, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
    };

};
//#endif
