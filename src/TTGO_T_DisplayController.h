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
#include <SPI.h>
#include <TFT_eSPI.h>

#if defined(ESP32)
#include <WiFi.h>
#include <esp_wifi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <Ethernet.h>
#endif

#include "displayController.h"

class SSD1306Brzo;
class OLEDDisplay;
class OLEDDisplayUiState;

typedef std::function<void(TFT_eSprite *tft, int16_t x, int16_t y)> RotatorFrameFunction;
typedef std::function<void(TFT_eSprite *tft)> RotatorOverlayFunction;

class SuperSimpleRotator
{
    TFT_eSPI *m_tft;
    std::vector<RotatorFrameFunction> m_frames;
    std::vector<RotatorOverlayFunction> m_underlays;
    size_t m_currentFrame;
    uint16_t m_timePerFrame;
    uint32_t m_startTimeCurrentFrame;
    uint16_t m_frameRate;
    uint32_t m_lastFrameTime;
    bool m_autoTransition;
    TFT_eSprite *m_spr;

protected:
    void render();

public:
    SuperSimpleRotator(TFT_eSPI *tft);
    ~SuperSimpleRotator();
    virtual int32_t handle();
    void setTimePerFrame(uint16_t timePerFrame)
    {
        m_timePerFrame = timePerFrame;
    }
    void setFrames(const std::vector<RotatorFrameFunction> &frames);
    void setUnderlays(const std::vector<RotatorOverlayFunction> &underlays);
    void switchToFrame(size_t currentFrame)
    {
        if (m_frames.size() < currentFrame)
        {
            m_currentFrame = currentFrame;
        }
    }
    void setAutoTransition(boolean autoTransition)
    {
        m_autoTransition = autoTransition;
    }

    TFT_eSprite* getSprite() const;
};

class TTGO_T_DisplayController : public DisplayController
{
protected:
    uint32_t m_lastMillis;
    TFT_eSPI *m_tft;
    SuperSimpleRotator *m_rotator;
    uint16_t m_palette[16];

public:
    TTGO_T_DisplayController();
    virtual ~TTGO_T_DisplayController();
    virtual void init();
    virtual int32_t handle();

private:
    ///////////////////////////////////////////////////////////////////////////
    //  UI Rendering
    ///////////////////////////////////////////////////////////////////////////
    void startScreen(TFT_eSprite *tft, int16_t x, int16_t y);

    void currentTemperatureSetting(TFT_eSprite *tft, int16_t x, int16_t y);
    void currentTemperatureSensor1(TFT_eSprite *tft, int16_t x, int16_t y);
    void currentTemperatureSensor2(TFT_eSprite *tft, int16_t x, int16_t y);

    void menuMain(TFT_eSprite *tft, int16_t x, int16_t y);
    void menuSetDesiredTemperature(TFT_eSprite *tft, int16_t x, int16_t y);
    void menuOverrideFan(TFT_eSprite *tft, int16_t x, int16_t y);

    void currentFanSpeed(TFT_eSprite *tft, int16_t x, int16_t y);
    void normalOverlayDisplay(TFT_eSprite *tft);

    // All screens for normal operation
    std::vector<RotatorFrameFunction> startScreens = {};
    std::vector<RotatorFrameFunction> normalRunScreens = {};
    std::vector<RotatorOverlayFunction> displayOverlay = {};
    std::vector<RotatorFrameFunction> menuScreens = {};

    std::unique_ptr<StateMachine> menuSequence;

    void toStringIp(const IPAddress &ip, char *ipAddress)
    {
        snprintf(ipAddress, 16, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
    };
};
//#endif
