#pragma once

// We can properly do better than this
#include <memory>
#include <array>
#include <cstddef>
#include <algorithm>

#include <bbqfanonly.h>
#include <statemachine.hpp>
#include <digitalinput.h>
#include <numericknob.h>
#include <SPI.h>
#include <TFT_eSPI.h>

#if defined(ESP32)
#include <WiFi.h>
#include <esp_wifi.h>
#elif defined(ESP8266)
#include <LittleFS.h>
#include <ESP8266WiFi.h>
#include <Ethernet.h>
#endif

#include "displayController.h"

class SSD1306Brzo;
class OLEDDisplay;
class OLEDDisplayUiState;

typedef std::function<bool(TFT_eSprite* tft, int16_t x, int16_t y)> RotatorFrameFunction;
typedef std::function<void(TFT_eSprite* tft)> RotatorOverlayFunction;
typedef std::function<float()> FloatProducer;


class ArrayWalker {
    const uint8_t* m_bmp;
    const uint32_t m_maxSize;
    uint32_t m_position;
public:
    ArrayWalker(const uint8_t* p_bmp, const uint32_t p_maxSize);
    bool eof() const;
    void seek(uint32_t offset);
    const uint8_t* data() const;
    bool read(uint8_t* buffer, std::size_t length);
    uint16_t read16();
    uint32_t read32();
};

class SuperSimpleRotator {
protected:
    TFT_eSPI* m_tft;
    std::vector<RotatorFrameFunction> m_frames;
    std::vector<RotatorOverlayFunction> m_underlays;
    size_t m_currentFrame;
    uint16_t m_timePerFrame;
    uint32_t m_startTimeCurrentFrame;
    uint16_t m_frameRate;
    uint32_t m_lastFrameTime;
    bool m_autoTransition;
    TFT_eSprite* m_spr;
    void render();

public:
    SuperSimpleRotator(TFT_eSPI* tft);
    ~SuperSimpleRotator();
    virtual int32_t handle();
    void setTimePerFrame(uint16_t timePerFrame);
    void setFrames(const std::vector<RotatorFrameFunction>& frames);
    void setUnderlays(const std::vector<RotatorOverlayFunction>& underlays);
    void switchToFrame(size_t currentFrame);
    void setAutoTransition(boolean autoTransition);
    TFT_eSprite* getSprite() const;
};

class TTGO_T_DisplayController : public DisplayController {
protected:
    uint32_t m_lastMillis;
    TFT_eSPI* m_tft;
    SuperSimpleRotator* m_rotator;
    uint16_t m_palette[16];
    // Temporary untill we can have the display functions handle object variables
    NumericInput* m_temperatureSetPointKnob;
    NumericInput* m_fanOverrideKnob;
    NumericInput* m_menuKnob;
    NumericInput* m_currentInput;
    uint32_t m_counter;

    State* STATE_STARTSCREEN = new State;
    State* STATE_WAITLOGO = new StateTimed{2500};
    State* STATE_CHANGETORUNSCREEN = new State;
    State* STATE_RUNSCREEN = new State;
    State* STATE_CHANGETOMENUSCREEN = new State;
    State* STATE_CHANGETOMENUBUTTONRELEASE = new State;
    State* STATE_SELECTMENUITEM = new State;
    State* STATE_SETTEMP = new State;
    State* STATE_SETFAN = new State;

public:
    TTGO_T_DisplayController();
    virtual ~TTGO_T_DisplayController();
    virtual void init();
    virtual int32_t handle();

private:
    ///////////////////////////////////////////////////////////////////////////
    //  UI Rendering
    ///////////////////////////////////////////////////////////////////////////
    bool startScreen(TFT_eSPI* m_tft, TFT_eSprite* tft, int16_t x, int16_t y);

    bool currentTemperatureSetting(TFT_eSprite* tft, int16_t x, int16_t y);
    bool currentTemperatureSensor(TFT_eSprite* tft, int16_t x, int16_t y, uint8_t num, float value);

    bool menuMain(TFT_eSprite* tft, int16_t x, int16_t y);
    bool menuSetDesiredTemperature(TFT_eSprite* tft, int16_t x, int16_t y);
    bool menuOverrideFan(TFT_eSprite* tft, int16_t x, int16_t y);

    bool currentFanSpeed(TFT_eSprite* tft, int16_t x, int16_t y);
    bool normalOverlayDisplay(TFT_eSprite* tft);

    // All screens for normal operation
    std::vector<RotatorFrameFunction> startScreens = {};
    std::vector<RotatorFrameFunction> normalRunScreens = {};
    std::vector<RotatorOverlayFunction> displayOverlay = {};
    std::vector<RotatorFrameFunction> menuScreens = {};

    std::unique_ptr<StateMachine> menuSequence;

    void toStringIp(const IPAddress& ip, char* ipAddress) {
        snprintf(ipAddress, 16, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
    };
};
