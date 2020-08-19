#include "TTGO_T_DisplayController.h"

#include <icons.h>
#include <PubSubClient.h>
#include "icons.h"
#include <digitalknob.h>
#include <memory>
#include <math.h>
#include <ventilator.h>
#include <pwmventilator.h>
#include <propertyutils.h>
#include <14segm.h>
#include <rotaryknob.h>

#define FRAMES_PER_SECOND 50
#define MILLIS_PER_FRAME (1000 / FRAMES_PER_SECOND)
#define GFXFF 1
#define SEG14 &DSEG14_Modern_Regular_40
constexpr uint8_t TFT_WIDTHD2 = TFT_WIDTH / 2;

/* Technical debt, all these externals should be passed to the display object somehow */
extern std::unique_ptr<BBQFanOnly> bbqController;
extern std::shared_ptr<TemperatureSensor> temperatureSensor1;
extern std::shared_ptr<TemperatureSensor> temperatureSensor2;
extern std::shared_ptr<Ventilator> ventilator1;
extern PubSubClient mqttClient;
extern DigitalKnob digitalKnob;
extern DigitalKnob rotary1;
extern DigitalKnob rotary2;
extern Properties bbqConfig;
typedef PropertyValue PV;
extern bool bbqConfigModified;


#define TEXT_WHERE_NO_BITMAP 1

constexpr uint8_t ITFT_BLACK = 0;
constexpr uint8_t ITFT_ORANGE = 1;
constexpr uint8_t ITFT_DARKGREEN = 2;
constexpr uint8_t ITFT_DARKCYAN = 3;
constexpr uint8_t ITFT_MAROON = 4;
constexpr uint8_t ITFT_PURPLE = 5;
constexpr uint8_t ITFT_OLIVE = 6;
constexpr uint8_t ITFT_DARKGREY = 7;
constexpr uint8_t ITFT_LIGHTGREY = 8;
constexpr uint8_t ITFT_BLUE = 9;
constexpr uint8_t ITFT_GREEN = 10;
constexpr uint8_t ITFT_SKYBLUE = 11;
constexpr uint8_t ITFT_RED = 12;
constexpr uint8_t ITFT_NAVY = 13;
constexpr uint8_t ITFT_YELLOW = 14;
constexpr uint8_t ITFT_WHITE = 15;

constexpr int16_t RIGHT_DISTANCE = 10;
constexpr int16_t LEFT_DISTANCE = 10;

// Takes around 35ms to render a full screen BMP
void drawBmp(TFT_eSPI* tft, int16_t x, int16_t y, const uint8_t* bmp, const uint32_t length) {

    if ((x >= tft->width()) || (y >= tft->height())) {
        return;
    }

    ArrayWalker walker(bmp, length);

    if (walker.read16() == 0x4D42) {
        walker.read32();
        walker.read32();
        uint32_t seekOffset = walker.read32();
        walker.read32();
        uint16_t w = walker.read32();
        uint16_t h = walker.read32();

        if ((walker.read16() == 1) && (walker.read16() == 24) && (walker.read32() == 0)) {
            y += h - 1;
            bool oldSwapBytes = tft->getSwapBytes();
            tft->setSwapBytes(true);
            walker.seek(seekOffset);
            uint16_t padding = (4 - ((w * 3) & 3)) & 3;
            uint8_t lineBuffer[w * 3 + padding];

            for (uint16_t row = 0; row < h; row++) {
                walker.read(lineBuffer, sizeof(lineBuffer));
                uint8_t*  bptr = lineBuffer;
                uint16_t* tptr = (uint16_t*)lineBuffer;

                // Convert 24 to 16 bit colours
                for (uint16_t col = 0; col < w; col++) {
                    *tptr++ = (*bptr++ >> 3) | ((*bptr++ & 0xFC) << 3) | ((*bptr++ & 0xF8) << 8);
                }

                tft->pushImage(x, y--, w, 1, (uint16_t*)lineBuffer);
            }

            tft->setSwapBytes(oldSwapBytes);
        }
    }
}

ArrayWalker::ArrayWalker(const uint8_t* p_bmp, const uint32_t p_maxSize) :
    m_bmp(p_bmp), m_maxSize(p_maxSize), m_position(0) {
}

bool ArrayWalker::eof() const {
    return m_position >= m_maxSize;
}

void ArrayWalker::seek(uint32_t offset) {
    m_position = offset;
}

const uint8_t* ArrayWalker::data() const {
    return &m_bmp[m_position];
}

bool ArrayWalker::read(uint8_t* buffer, std::size_t length) {
    std::memcpy(buffer, m_bmp + m_position, length);
    m_position += length;
    return true;
}

uint16_t ArrayWalker::read16() {
    uint16_t result;
    ((uint8_t*)&result)[0] = m_bmp[m_position++];  // LSB
    ((uint8_t*)&result)[1] = m_bmp[m_position++];
    return result;
}

uint32_t ArrayWalker::read32() {
    uint32_t result;
    ((uint8_t*)&result)[0] = m_bmp[m_position++];  // LSB
    ((uint8_t*)&result)[1] = m_bmp[m_position++];
    ((uint8_t*)&result)[2] = m_bmp[m_position++];  // LSB
    ((uint8_t*)&result)[3] = m_bmp[m_position++];
    return result;
}

SuperSimpleRotator::SuperSimpleRotator(TFT_eSPI* tft) : m_tft(tft),
    m_currentFrame(-1),
    m_timePerFrame(2500),
    m_startTimeCurrentFrame(0),
    m_frameRate(1000 / 10),
    m_lastFrameTime(0),
    m_autoTransition(true),
    m_spr(new TFT_eSprite(m_tft)) {
    // TODO, make these configurable. Now we just assume horizontal 16 bit color display
    m_spr->setColorDepth(4);
    m_spr->createSprite(TFT_HEIGHT, TFT_WIDTH);
}

SuperSimpleRotator::~SuperSimpleRotator() {
    delete m_spr;
}

void SuperSimpleRotator::render() {
    m_spr->fillScreen(TFT_BLACK);
    m_spr->setTextDatum(TL_DATUM);

    for (auto& underlay : m_underlays) {
        underlay(m_spr);
    }

    bool pushSprite = true;

    if (m_frames.size() > 0) {
        pushSprite = pushSprite && m_frames.at(m_currentFrame)(m_spr, 0, 0);
    }

    if (pushSprite) {
        m_spr->pushSprite(0, 0);
    }
}

int32_t SuperSimpleRotator::handle() {
    const uint32_t time = millis();

    // Ensure we only render at framerate
    if (time - m_lastFrameTime > m_frameRate) {
        m_lastFrameTime = time;
        // render frame:
        render();
    }

    // Decide when to move to next frame
    if (m_autoTransition && (time - m_startTimeCurrentFrame > m_timePerFrame)) {
        m_startTimeCurrentFrame = time;
        m_currentFrame++;

        if (m_currentFrame >= m_frames.size()) {
            m_currentFrame = 0;
        }
    }

    return 0;
}

void SuperSimpleRotator::setFrames(const std::vector<RotatorFrameFunction>& frames) {
    m_frames = frames;
    m_currentFrame = 0;
}

void SuperSimpleRotator::setUnderlays(const std::vector<RotatorOverlayFunction>& underlays) {
    m_underlays = underlays;
}

TFT_eSprite* SuperSimpleRotator::getSprite() const {
    return m_spr;
}

void SuperSimpleRotator::setTimePerFrame(uint16_t timePerFrame) {
    m_timePerFrame = timePerFrame;
}

void SuperSimpleRotator::switchToFrame(size_t nextFrame) {
    if (nextFrame >= 0 && nextFrame < m_frames.size()) {
        m_currentFrame = nextFrame;
    }
}
void SuperSimpleRotator::setAutoTransition(boolean autoTransition) {
    m_autoTransition = autoTransition;
}

TTGO_T_DisplayController::TTGO_T_DisplayController() : DisplayController(),
    m_lastMillis(0),
    m_tft(new TFT_eSPI()),
    m_rotator(new SuperSimpleRotator(m_tft)),
    m_temperatureSetPointKnob(nullptr),
    m_fanOverrideKnob(nullptr),
    m_menuKnob(nullptr),
    m_currentInput(nullptr),
    m_counter(false) {
    m_palette[ITFT_BLACK] = TFT_BLACK;
    m_palette[ITFT_ORANGE] = TFT_ORANGE;
    m_palette[ITFT_DARKGREEN] = TFT_DARKGREEN;
    m_palette[ITFT_DARKCYAN] = TFT_DARKCYAN;
    m_palette[ITFT_MAROON] = TFT_MAROON;
    m_palette[ITFT_PURPLE] = TFT_PURPLE;
    m_palette[ITFT_OLIVE] = TFT_OLIVE;
    m_palette[ITFT_DARKGREY] = TFT_DARKGREY;
    m_palette[ITFT_LIGHTGREY] = TFT_LIGHTGREY;
    m_palette[ITFT_BLUE] = TFT_BLUE;
    m_palette[ITFT_GREEN] = TFT_GREEN;
    m_palette[ITFT_SKYBLUE] = TFT_SKYBLUE;
    m_palette[ITFT_RED] = TFT_RED;
    m_palette[ITFT_NAVY] = TFT_NAVY;
    m_palette[ITFT_YELLOW] = TFT_YELLOW;
    m_palette[ITFT_WHITE] = TFT_WHITE;
    m_rotator->getSprite()->createPalette(m_palette, 16);
}

TTGO_T_DisplayController::~TTGO_T_DisplayController() {
    delete m_tft;
}

void TTGO_T_DisplayController::init() {

    m_temperatureSetPointKnob = new RotaryKnob(&rotary1, &rotary2, bbqController->setPoint(), 90, 240, .5f);
    m_fanOverrideKnob = new RotaryKnob(&rotary1, &rotary2, ventilator1->speedOverride(), -1, 100, 1);
    m_menuKnob = new RotaryKnob(&rotary1, &rotary2, 0, 0, 2, 0.2);

    startScreens = {
        [&](TFT_eSprite * tft, int16_t x, int16_t y) {
            return startScreen(m_tft, tft, x, y);
        }
    };

    normalRunScreens = {
        [&](TFT_eSprite * tft, int16_t x, int16_t y) { return currentTemperatureSetting(tft, x, y); },
        [&](TFT_eSprite * tft, int16_t x, int16_t y) { return currentTemperatureSensor(tft, x, y, 1, temperatureSensor1->get()); },
        [&](TFT_eSprite * tft, int16_t x, int16_t y) { return currentTemperatureSensor(tft, x, y, 2, temperatureSensor2->get()); },
        [&](TFT_eSprite * tft, int16_t x, int16_t y) {
            return currentFanSpeed(tft, x, y);
        }
    };

    displayOverlay = {
        [&](TFT_eSprite * tft) {
            normalOverlayDisplay(tft);
        }
    };

    menuScreens = {
        [&](TFT_eSprite * tft, int16_t x, int16_t y) { return menuMain(tft, x, y); },
        [&](TFT_eSprite * tft, int16_t x, int16_t y) { return menuSetDesiredTemperature(tft, x, y); },
        [&](TFT_eSprite * tft, int16_t x, int16_t y) {
            return menuOverrideFan(tft, x, y);
        }
    };

    State* STATE_STARTSCREEN;
    State* STATE_WAITLOGO;
    State* STATE_CHANGETORUNSCREEN;
    State* STATE_RUNSCREEN;
    State* STATE_CHANGETOMENUSCREEN;
    State* STATE_SELECTMENUITEM;
    State* STATE_SETTEMP;
    State* STATE_SETFAN;
    State* STATE_CHANGETOMENUBUTTONRELEASE;

    // 0
    STATE_STARTSCREEN = new State([&]() {
        m_rotator->setUnderlays(displayOverlay);
        m_rotator->setFrames(startScreens);
        return 1;
    });

    // 1
    STATE_WAITLOGO = new StateTimed((2500), [&]() {
        // Display splash screen
        return 2;
    });

    // 2
    STATE_CHANGETORUNSCREEN = new State([&]() {
        m_rotator->setFrames(normalRunScreens);
        m_rotator->setAutoTransition(true);
        m_currentInput = nullptr;
        return 3;
    });

    // 3
    STATE_RUNSCREEN = new State([&]() {
        if (digitalKnob.isEdgeUp()) {
            return 4;
        }

        return 3;
    });

    // 4
    STATE_CHANGETOMENUSCREEN = new State([&]() {
        digitalKnob.reset();
        m_rotator->setFrames(menuScreens);
        m_rotator->setAutoTransition(false);
        m_menuKnob->value(0);
        m_currentInput = m_menuKnob;
        return 6;
    });

    // 5
    STATE_CHANGETOMENUBUTTONRELEASE = new State([&]() {
        //        if (digitalKnob.current() == false) {
        // Clears the internal status so we don´t get a false click later
        //            digitalKnob.reset();
        //            return 4;
        //        }

        return 4;
    });

    // 6
    STATE_SELECTMENUITEM = new State([&]() {
        if (digitalKnob.isEdgeUp()) {
            uint8_t menu = ((int)round(m_menuKnob->value()));
            m_rotator->switchToFrame(menu);

            switch (menu) {
                case 0:
                    return 2;

                case 1:
                    m_temperatureSetPointKnob->value(bbqController->setPoint());
                    m_currentInput = m_temperatureSetPointKnob;
                    return 7;

                case 2:
                    m_fanOverrideKnob->value(ventilator1->speedOverride());
                    m_currentInput = m_fanOverrideKnob;
                    return 8;
            };
        }

        return 6;
    });

    // 7
    STATE_SETTEMP = new State([&]() {
        if (digitalKnob.isEdgeUp()) {
            float value = round(m_temperatureSetPointKnob->value() * 2.0f) / 2.0f;
            bbqConfig.put("setPoint", PV(value));
            bbqController->setPoint(value);
            bbqConfigModified = true;
            return 4;
        }

        return 7;
    });

    // 8
    STATE_SETFAN = new State([&]() {
        if (digitalKnob.isEdgeUp()) {
            float value = round(m_fanOverrideKnob->value());
            ventilator1->speedOverride(value);
            return 4;
        }

        return 8;
    });

    menuSequence.reset(new StateMachine({
        STATE_STARTSCREEN,               // 0
        STATE_WAITLOGO,                  // 1
        STATE_CHANGETORUNSCREEN,         // 2
        STATE_RUNSCREEN,                 // 3
        STATE_CHANGETOMENUSCREEN,        // 4
        STATE_CHANGETOMENUBUTTONRELEASE, // 5
        STATE_SELECTMENUITEM,            // 6
        STATE_SETTEMP,                   // 7
        STATE_SETFAN                     // 8
    }));

    //Set up the display
    m_tft->init();
    m_tft->setRotation(1);
    m_tft->fillScreen(TFT_BLACK);
    m_tft->setCursor(0, 0);
    m_tft->setTextColor(ITFT_WHITE, TFT_BLACK);

    // Start menu
    menuSequence->start();
    menuSequence->handle(); // We call handle to ensure first frame is rendered directly
}

int32_t TTGO_T_DisplayController::handle() {
    uint32_t currentMillis = millis();

    if (m_currentInput != nullptr) {
        m_currentInput->handle();
    }

    m_rotator->handle();

    if ((currentMillis - m_lastMillis) >= MILLIS_PER_FRAME) {
        m_lastMillis = currentMillis;
        menuSequence->handle();
        m_counter++;
    }

    return MILLIS_PER_FRAME;
}

///////////////////////////////////////////////////////////////////
//////////////////////// DISPLAY RENDERING ////////////////////////
///////////////////////////////////////////////////////////////////
bool TTGO_T_DisplayController::startScreen(TFT_eSPI* m_tft, TFT_eSprite* ptft, int16_t x, int16_t y) {
    drawBmp(m_tft, 0, 0, splash240_bmp, splash240_bmp_len);
    return false;
}

bool TTGO_T_DisplayController::currentTemperatureSetting(TFT_eSprite* tft, int16_t x, int16_t y) {
    char buffer[16];
    sprintf(buffer, "%.1f°C", bbqController->setPoint());
    tft->setFreeFont(SEG14);
    tft->setTextDatum(CR_DATUM);
    tft->setTextColor(ITFT_WHITE);
    tft->drawString(buffer, x + TFT_HEIGHT - RIGHT_DISTANCE, y + TFT_WIDTHD2, GFXFF);
    tft->drawXBitmap(x + LEFT_DISTANCE, y + TFT_WIDTHD2 - knob48_height / 2, knob48_bits, knob48_width, knob48_height, ITFT_WHITE);
    return true;
}

bool TTGO_T_DisplayController::currentTemperatureSensor(TFT_eSprite* tft, int16_t x, int16_t y, uint8_t num, float value) {
    char buffer[16];
    tft->setFreeFont(SEG14);
    tft->setTextDatum(CR_DATUM);
    sprintf(buffer, "%d: %.1f°C", num, value);
    tft->setTextColor(ITFT_ORANGE);
    tft->drawString(buffer, x + TFT_HEIGHT - RIGHT_DISTANCE, y + TFT_WIDTHD2, GFXFF);
    tft->drawXBitmap(x + LEFT_DISTANCE, y + TFT_WIDTHD2 - thermometer48_height / 2, thermometer48_bits, thermometer48_width, thermometer48_height, ITFT_ORANGE);
    return true;
}

bool TTGO_T_DisplayController::currentFanSpeed(TFT_eSprite* tft, int16_t x, int16_t y) {
    tft->setTextDatum(CR_DATUM);

    if (!ventilator1->isOverride()) {
        tft->setTextColor(ITFT_GREEN);
        tft->drawString("Auto", x + TFT_HEIGHT, y + 30, 4);
    } else {
        tft->setTextColor(ITFT_ORANGE);
        tft->drawString("Manual", x + TFT_HEIGHT, y + 30 + 48 + 26, 4);
    }

    char buffer[16];
    float value = ventilator1->speed();

    if (value < 0.1) {
        sprintf(buffer, "Off");
    } else {
        sprintf(buffer, "%3.0f%%", value);
    }

    tft->setFreeFont(SEG14);
    tft->setTextColor(ITFT_SKYBLUE);
    tft->drawString(buffer, x + TFT_HEIGHT - RIGHT_DISTANCE /* x-advance 0 */, y + TFT_WIDTHD2, GFXFF);
    tft->drawXBitmap(x + LEFT_DISTANCE, y + TFT_WIDTHD2 - fan48_height / 2, fan48_bits, fan48_width, fan48_height, ITFT_SKYBLUE);
    return true;
}

bool TTGO_T_DisplayController::menuMain(TFT_eSprite* tft, int16_t x, int16_t y) {
    uint8_t menu = ((int)round(m_menuKnob->value()));

    int8_t markerPos = -6;
    const uint8_t yOffset = 24;
    const uint8_t extraOffset = 40;

    switch (menu) {
        case 0:
            markerPos = yOffset * 0;
            break;

        case 1:
            markerPos = yOffset * 1;
            break;

        case 2:
            markerPos = yOffset * 2;
            break;
    }

    tft->setTextColor(ITFT_LIGHTGREY);
    tft->drawString("Back", x + LEFT_DISTANCE + 40, y + yOffset * 0 + extraOffset, 4);
    tft->drawString("Set Temperature", x + LEFT_DISTANCE + 40, y + yOffset * 1 + extraOffset, 4) ;
    tft->drawString("Override fan", x + LEFT_DISTANCE + 40, y + yOffset * 2 + extraOffset, 4);

    tft->setTextColor(m_counter % 26 < 13 ? ITFT_WHITE : ITFT_LIGHTGREY);
    tft->drawString("o", x + LEFT_DISTANCE + 20, y + markerPos + extraOffset, 4);

    return true;
}

bool TTGO_T_DisplayController::menuSetDesiredTemperature(TFT_eSprite* tft, int16_t x, int16_t y) {
    float value = round(m_temperatureSetPointKnob->value() * 2.0f) / 2.0f;
    char buffer[16];
    sprintf(buffer, "%.1f°C", value);

    tft->setFreeFont(SEG14);
    tft->setTextDatum(CR_DATUM);
    tft->setTextColor(m_counter % 26 < 13 ? ITFT_ORANGE : ITFT_LIGHTGREY);
    tft->drawString(buffer, x + TFT_HEIGHT - RIGHT_DISTANCE, y + TFT_WIDTHD2, GFXFF);
    tft->drawXBitmap(x + LEFT_DISTANCE, y + TFT_WIDTHD2 - thermometer48_height / 2, thermometer48_bits, thermometer48_width, thermometer48_height, ITFT_ORANGE);

    return true;
}

bool TTGO_T_DisplayController::menuOverrideFan(TFT_eSprite* tft, int16_t x, int16_t y) {
    int16_t value = round(m_fanOverrideKnob->value());
    char buffer[16];

    if (value == -1) {
        sprintf(buffer, "Auto");
    } else if (value == 0) {
        sprintf(buffer, "Off");
    } else {
        sprintf(buffer, "%i%%", value);
    }

    tft->setTextDatum(CR_DATUM);
    tft->setFreeFont(SEG14);
    tft->setTextColor(ITFT_SKYBLUE);
    tft->drawXBitmap(x + LEFT_DISTANCE, y + TFT_WIDTHD2 - fan48_height / 2, fan48_bits, fan48_width, fan48_height, ITFT_SKYBLUE);
    tft->setTextColor(m_counter % 26 < 13 ? ITFT_SKYBLUE : ITFT_LIGHTGREY);
    tft->drawString(buffer, x + TFT_HEIGHT - RIGHT_DISTANCE /* x-advance 0 */, y + TFT_WIDTHD2, GFXFF);

    return true;
}

bool TTGO_T_DisplayController::normalOverlayDisplay(TFT_eSprite* tft) {
    char buffer[16];

    tft->fillSprite(ITFT_BLACK);

    // Current temperature speed
    sprintf(buffer, "%.1f°C", temperatureSensor1->get());
    tft->setTextDatum(TR_DATUM);
    tft->setTextColor(ITFT_WHITE);
    tft->drawString(buffer, TFT_HEIGHT, 0, 2);

    uint8_t xPos = 10;

    tft->setTextDatum(TL_DATUM);

    if (WiFi.status() == WL_CONNECTED) {
        tft->setTextColor(ITFT_GREEN);
    } else {
        tft->setTextColor(ITFT_RED);
    }

    xPos += tft->drawString("Wifi", xPos, 0, 2) + 10;

    if (mqttClient.connected()) {
        tft->setTextColor(ITFT_GREEN);
    } else {
        tft->setTextColor(ITFT_RED);
    }

    xPos += tft->drawString("Mqtt", xPos, 0, 2) + 10;

    /*
    if (bbqController->lidOpen()) {
        tft->setTextColor(ITFT_ORANGE);
        xPos += tft->drawString("Lid Open", xPos, 0, 2) + 10;
    } else {
        tft->setTextColor(ITFT_GREEN);
        xPos += tft->drawString("Lid Closed", xPos, 0, 2) + 10;
    }
    */

    return true;
}
