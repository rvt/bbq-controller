/*
 */
#include <memory>
#include <cstring>
#include <vector>

#include "debug.h"

#include <ESP8266WiFi.h>  // https://github.com/esp8266/Arduino
#include <ESP8266mDNS.h>

#include <brzo_i2c.h>
#include "SSD1306Brzo.h"
#include <OLEDDisplayUi.h>
#include <propertyutils.h>
#include <optparser.h>
#include <utils.h>
#include <icons.h>
#include <crceeprom.h>
#include <pwmventilator.h>
#include <makeString.h>

#include <max31865sensor.h>
#include <max31855sensor.h>
#include <PubSubClient.h> // https://github.com/knolleary/pubsubclient/releases/tag/v2.6


#include <ArduinoOTA.h>
#include <ESP_EEPROM.h>
#include <settings.h>
#include <analogin.h>
#include <Fuzzy.h>

#include "settingsdto.h"

#include <bbqfanonly.h>
#include <bbq.h>
#include <demo.h>
#include <statemachine.h>


// of transitions
volatile uint32_t transitionCounter = 1;

// Number calls per second we will be handling
#define FRAMES_PER_SECOND        10
#define EFFECT_PERIOD_CALLBACK   (1000 / FRAMES_PER_SECOND)

// Keep track when the last time we ran the effect state changes
volatile uint32_t effectPeriodStartMillis = 0;


typedef PropertyValue PV ;
Properties properties;
SSD1306Brzo display(0x3c, WIRE_SDA, WIRE_SCL);
OLEDDisplayUi ui(&display);

std::unique_ptr<BBQFanOnly> bbqController(nullptr);
std::shared_ptr<TemperatureSensor> temperatureSensor1(nullptr);
std::shared_ptr<TemperatureSensor> temperatureSensor2(nullptr);
std::shared_ptr<Ventilator> ventilator1(nullptr);

// START: For demo/test mode
MockedTemperature* mockedTemp1 = new MockedTemperature(20.0);
MockedTemperature* mockedTemp2 = new MockedTemperature(30.0);
// END: For demo/test mode
float analogKnobTemperatureSetPoint; // Temperature setpoint from analog knob

std::unique_ptr<NumericInput> analogIn(nullptr);

// Settings
SettingsDTO settingsDTO;

uint16_t lastUpdateCRC = 0;

// Eeprom storage
// wait 500ms after last commit, then commit no more often than every 30s
Settings eepromSaveHandler(
    500,
    30000,
[]() {

    CRCEEProm::write(0, *settingsDTO.data());
    EEPROM.commit();
},
[]() {
    return settingsDTO.modified();
}
);

// MQTT Storage
// mqtt updates as quickly as possible with a maximum frequence of MQTT_STATE_UPDATE_DELAY
void publishToMQTT(const char* topic, const char* payload);
Settings mqttSaveHandler(
    1,
    MQTT_STATE_UPDATE_DELAY,
[]() {

    std::string configString = settingsDTO.getConfigString();

    if (configString.size() > 0) {
        Serial.print("\n");
        Serial.print(configString.c_str());
        Serial.print("\n");
        Serial.print(properties.get("mqttConfigStateTopic").getCharPtr());
        Serial.print("\n");
        publishToMQTT(properties.get("mqttConfigStateTopic").getCharPtr(), configString.c_str());
    }

},
[]() {
    return settingsDTO.modified();
}
);




State* BOOTSEQUENCESTART;
//State* SETUPSERIAL;
State* DELAYEDMQTTCONNECTION;
State* TESTMQTTCONNECTION;
State* CONNECTMQTT;
State* PUBLISHONLINE;
State* SUBSCRIBECOMMANDTOPIC;
State* WAITFORCOMMANDCAPTURE;

std::unique_ptr<StateMachine> bootSequence(nullptr);


#if defined(TLS)
WiFiClientSecure wifiClient;
#else
WiFiClient wifiClient;
#endif
PubSubClient mqttClient(wifiClient);

///////////////////////////////////////////////////////////////////////////
//  SSL/TLS
///////////////////////////////////////////////////////////////////////////
/*
  Function called to verify the fingerprint of the MQTT server certificate
*/
#if defined(TLS)
void verifyFingerprint() {
    DEBUG_PRINT(F("INFO: Connecting to "));
    DEBUG_PRINTLN(MQTT_SERVER);

    if (!wifiClient.connect(MQTT_SERVER, MQTT_PORT)) {
        DEBUG_PRINTLN(F("ERROR: Connection failed. Halting execution"));
        delay(1000);
        ESP.reset();
    }

    if (wifiClient.verify(TLS_FINGERPRINT, MQTT_SERVER)) {
        DEBUG_PRINTLN(F("INFO: Connection secure"));
    } else {
        DEBUG_PRINTLN(F("ERROR: Connection insecure! Halting execution"));
        delay(1000);
        ESP.reset();
    }
}
#endif

///////////////////////////////////////////////////////////////////////////
//  Utilities
///////////////////////////////////////////////////////////////////////////


/**
 * Publish a message to mqtt
 */
void publishToMQTT(const char* topic, const char* payload) {
    if (mqttClient.publish(topic, payload, true)) {
        DEBUG_PRINT(F("INFO: MQTT message publish succeeded. Topic: "));
        DEBUG_PRINT(topic);
        DEBUG_PRINT(F(". Payload: "));
        DEBUG_PRINTLN(payload);
    } else {
        Serial.println("Failed to publish");
        DEBUG_PRINTLN(F("ERROR: MQTT message publish failed, either connection lost, or message too large"));
    }
}


/**
 * Send a command to the cmdHandler
 */
void handleCmd(const char* topic, const char* p_payload) {
    long m_mqttSubscriberTopicStrLength = properties.get("mqttSubscriberTopicStrLength").getLong();
    auto topicPos = topic + m_mqttSubscriberTopicStrLength;
    Serial.println(topic);
    Serial.println(p_payload);


    // Look for a temperature setPoint topic
    if (strstr(topicPos, MQTT_CONFIG_TOPIC) != nullptr) {
        BBQFanOnlyConfig config = bbqController->config();
        float temperature = 0;
        OptParser::get(p_payload, [&config, &temperature](OptValue values) {

            if (strcmp(values.key(), "sp") == 0) {
                temperature = values.asFloat();
            }

            // Fan 1 override ( we don´t want this as an settings so if we loose MQTT connection we can always unplug)
            if (strcmp(values.key(), "f1o") == 0) {
                ventilator1->speedOverride(values.asFloat());
            }

            if (strcmp(values.key(), "ta") == 0) {
                config.temp_alpha = values.asFloat();
            }

            config.fan_low = getConfigArray("fl", values.key(), values.asChar(), config.fan_low);
            config.fan_medium = getConfigArray("fm", values.key(), values.asChar(), config.fan_medium);
            config.fan_high = getConfigArray("fh", values.key(), values.asChar(), config.fan_high);

            config.temp_error_low = getConfigArray("tel", values.key(), values.asChar(), config.temp_error_low);
            config.temp_error_medium = getConfigArray("tem", values.key(), values.asChar(), config.temp_error_medium);
            config.temp_error_hight = getConfigArray("teh", values.key(), values.asChar(), config.temp_error_hight);

            config.temp_change_fast = getConfigArray("tcf", values.key(), values.asChar(), config.temp_change_fast);
        });

        if (temperature > 15 && temperature < 260) {
            bbqController->setPoint(temperature);
            settingsDTO.data()->setPoint = temperature;
        }

        // Copy to settings
        settingsDTO.data()->temp_alpha = config.temp_alpha;
        settingsDTO.data()->fan_low = config.fan_low;
        settingsDTO.data()->fan_medium = config.fan_medium;
        settingsDTO.data()->fan_high = config.fan_high;
        settingsDTO.data()->temp_error_low = config.temp_error_low;
        settingsDTO.data()->temp_error_medium = config.temp_error_medium;
        settingsDTO.data()->temp_error_hight = config.temp_error_hight;
        settingsDTO.data()->temp_change_fast = config.temp_change_fast;

        // Update the bbqController with new values
        bbqController->config(config);
        bbqController->init();
    }

    // Dummy data topic during testing
    // With ot we can simulate a oven temperature
    // BBQ/xxxxxx/dummy
#ifdef DEMO_MODE

    if (strstr(topicPos, TEMPERATURE_DUMMY_TOPIC) != nullptr) {
        OptParser::get(p_payload, [](OptValue v) {
            if (strcmp(v.key(), "t1") == 0) {
                mockedTemp1->set(v.asFloat());
            }

            if (strcmp(v.key(), "t2") == 0) {
                mockedTemp2->set(v.asFloat());
            }
        });
    }

#endif
}

///////////////////////////////////////////////////////////////////////////
//  UI Rendering
///////////////////////////////////////////////////////////////////////////

void updatdisplayFrame1(OLEDDisplay* display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
    char buffer[32];
    // Set Temperature
    display->setFont(ArialMT_Plain_24);
    display->setTextAlignment(TEXT_ALIGN_LEFT);
    sprintf(buffer, "%.1f°C", bbqController->setPoint());
    display->drawString(x + 0 + knob_width + 4, y + 20, buffer);
    display->drawXbm(0, y + 20, knob_width, knob_height, knob_bits);
}

void disdplayOverlay(OLEDDisplay* display, OLEDDisplayUiState* state) {
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

void updatdisplayFrame2(OLEDDisplay* display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
    char buffer[32];
    // Current temperature speed
    display->setFont(ArialMT_Plain_24);
    display->setTextAlignment(TEXT_ALIGN_LEFT);
    sprintf(buffer, "1 %.1f°C", temperatureSensor1->get());
    display->drawString(x + 0 + thermometer_width + 4, y + 20, buffer);
    display->drawXbm(0, y + 20, thermometer_width, thermometer_height, (uint8_t*)thermometer_bits);
}

void updatdisplayFrame3(OLEDDisplay* display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
    char buffer[32];
    // Current temperature speed
    display->setFont(ArialMT_Plain_24);
    display->setTextAlignment(TEXT_ALIGN_LEFT);
    sprintf(buffer, "2 %.1f°C", temperatureSensor2->get());
    display->drawString(x + 0 + thermometer_width + 4, y + 20, buffer);
    display->drawXbm(0, y + 20, thermometer_width, thermometer_height, (uint8_t*)thermometer_bits);
}

void updatdisplayFrame4(OLEDDisplay* display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
    char buffer[32];
    // Ventilator speed
    display->setFont(ArialMT_Plain_24);
    display->setTextAlignment(TEXT_ALIGN_LEFT);
    sprintf(buffer, "%3.0f%%", ventilator1->speed());
    display->drawString(x + 0 + fan_width + 4, y + 20, buffer);
    display->drawXbm(0, y + 20, fan_width, fan_height, fan_bits);
}

FrameCallback frames[] = { updatdisplayFrame1, updatdisplayFrame2, updatdisplayFrame3, updatdisplayFrame4};
int numberOfFrames = *(&numberOfFrames + 1) - numberOfFrames;


/*
* Publish current status
* to = temperature oven
* sp = set Point
* f1 = Speed of fan 1
* lo = Lid open alert
* lc = Low charcoal alert
*/
void publishStatus() {
    const char* format = "to=%.2f t2=%.2f sp=%.2f f1=%.2f lo=%i lc=%i f1o=%.1f";
    char buffer[(4 + 6) * 6 + 16]; // 10 characters per item times extra items to be sure
    sprintf(buffer, format,
            temperatureSensor1->get(),
            temperatureSensor2->get(),
            bbqController->setPoint(),
            ventilator1->speed(),
            bbqController->lidOpen(),
            bbqController->lowCharcoal(),
            ventilator1->speedOverride()
           );

    // Quick hack to only update when data actually changed
    uint16_t thisCrc = CRCEEProm::crc16((uint8_t*)buffer, strlen(buffer));

    if (thisCrc != lastUpdateCRC) {
        publishToMQTT(properties.get("mqttStatusTopic").getCharPtr(), buffer);
    }

    lastUpdateCRC = thisCrc;
}

///////////////////////////////////////////////////////////////////////////
//  WiFi
///////////////////////////////////////////////////////////////////////////

void setupWiFi(const Properties& props) {
    auto mqttClientID = props.get("mqttClientID").getCharPtr();
    auto wifi_ssid = props.get("wifi_ssid").getCharPtr();
    auto wifi_password = props.get("wifi_password").getCharPtr();
    WiFi.hostname(mqttClientID);
    delay(10);
    Serial.print(F("INFO: Connecting to: "));
    Serial.println(wifi_ssid);
    WiFi.mode(WIFI_STA);
    WiFi.begin(wifi_ssid, wifi_password);
    randomSeed(micros());
    MDNS.begin(mqttClientID);
}

void loadConfiguration(Properties& props) {
    // 00FF1234
    const char* chipId = makeCString("%08X", ESP.getChipId());
    // BBQ00FF1234
    props.put("mqttClientID", PV(makeCString(HOSTNAME_TEMPLATE, chipId)));
    // BBQ/00FF1234
    const char* mqttTopicPrefix = makeCString(MQTT_TOPIC_PREFIX_TEMPLATE, MQTT_PREFIX, chipId);
    props.put("mqttTopicPrefix", PV(mqttTopicPrefix));
    // BBQ/00FF1234/lastwill
    props.put("mqttLastWillTopic", PV(makeCString(MQTT_LASTWILL_TOPIC_TEMPLATE, mqttTopicPrefix)));
    // BBQ/00FF1234/config/state
    props.put("mqttConfigStateTopic", PV(makeCString(MQTT_CONFIG_TOPIC_STATE_TEMPLATE, mqttTopicPrefix)));
    // BBQ/00FF1234/status
    props.put("mqttStatusTopic", PV(makeCString(MQTT_STATUS_TOPIC_TEMPLATE, mqttTopicPrefix)));
    //  BBQ/00FF1234/+
    const char* mqttSubscriberTopic = makeCString(MQTT_SUBSCRIBER_TOPIC_TEMPLATE, mqttTopicPrefix);
    props.put("mqttSubscriberTopic", PV(mqttSubscriberTopic));
    // Calculate length of the subcriber topic
    props.put("mqttSubscriberTopicStrLength", PV((int32_t)std::strlen(mqttSubscriberTopic) - 2));
    // friendlyName : BBQ Controller 00FF1234
    props.put("friendlyName", PV(makeCString("BBQ Controller %s", chipId)));
    // Wigi username and password
    props.put("wifi_ssid", PV(WIFI_SSID));
    props.put("wifi_password", PV(WIFI_PASSWORD));
    // mqtt server and port
    props.put("mqtt_server", PV(MQTT_SERVER));
    props.put("mqtt_port", PV(MQTT_PORT));
    delete chipId;
}

/**
 * Start OTA
 */
void startOTA() {
    // Start OTA
    ArduinoOTA.setHostname(properties.get("mqttClientID").getCharPtr());
    ArduinoOTA.onStart([]() {
        DEBUG_PRINTLN(F("OTA Beginning"));
        // turn PWM off
    });
    ArduinoOTA.onError([](ota_error_t error) {
        DEBUG_PRINT("ArduinoOTA Error[");
        DEBUG_PRINT(error);
        DEBUG_PRINT("]: ");

        if (error == OTA_AUTH_ERROR) {
            DEBUG_PRINTLN(F("Auth Failed"));
        } else if (error == OTA_BEGIN_ERROR) {
            DEBUG_PRINTLN(F("Begin Failed"));
        } else if (error == OTA_CONNECT_ERROR) {
            DEBUG_PRINTLN(F("Connect Failed"));
        } else if (error == OTA_RECEIVE_ERROR) {
            DEBUG_PRINTLN(F("Receive Failed"));
        } else if (error == OTA_END_ERROR) {
            DEBUG_PRINTLN(F("End Failed"));
        }
    });
    ArduinoOTA.begin();
    ArduinoOTA.handle();
}

///////////////////////////////////////////////////////////////////////////
//  SETUP() AND LOOP()
///////////////////////////////////////////////////////////////////////////

OverlayCallback overlays[] = { disdplayOverlay };

void setup() {
    // Enable serial port
    Serial.begin(115200);
    delay(50);
    Serial.println(F("Starting"));
    Serial.print(F("Hostname: "));
    // setup Strings
    loadConfiguration(properties);
    Serial.println(properties.get("mqttClientID").getCharPtr());
    BOOTSEQUENCESTART = new State([]() {
        return TESTMQTTCONNECTION;
    });
    DELAYEDMQTTCONNECTION = new StateTimed(1500, []() {
        return TESTMQTTCONNECTION;
    });
    TESTMQTTCONNECTION = new State([]() {
        if (mqttClient.connected())  {
            if (WiFi.status() != WL_CONNECTED) {
                mqttClient.disconnect();
            }

            return DELAYEDMQTTCONNECTION;
        }

        return CONNECTMQTT;
    });
    CONNECTMQTT = new State([]() {
        if (mqttClient.connect(
                properties.get("mqttClientID").getCharPtr(),
                MQTT_USER,
                MQTT_PASS,
                properties.get("mqttLastWillTopic").getCharPtr(),
                0, 1, MQTT_LASTWILL_OFFLINE)) {
            return PUBLISHONLINE;
        }

        DEBUG_PRINTLN(F("ERROR: The connection to the MQTT broker failed"));
        DEBUG_PRINT(F("Username: "));
        DEBUG_PRINTLN(MQTT_USER);
        DEBUG_PRINT(F("Broker: "));
        DEBUG_PRINTLN(MQTT_SERVER);
        return DELAYEDMQTTCONNECTION;
    });
    PUBLISHONLINE = new State([]() {
        publishToMQTT(
            properties.get("mqttLastWillTopic").getCharPtr(),
            MQTT_LASTWILL_ONLINE);
        return SUBSCRIBECOMMANDTOPIC;
    });
    SUBSCRIBECOMMANDTOPIC = new State([]() {
        if (mqttClient.subscribe(properties.get("mqttSubscriberTopic").getCharPtr(), 0)) {
            Serial.println(properties.get("mqttSubscriberTopic").getCharPtr());
            return WAITFORCOMMANDCAPTURE;
        }

        DEBUG_PRINT(F("ERROR: Failed to connect to topic : "));
        mqttClient.disconnect();
        return DELAYEDMQTTCONNECTION;
    });
    WAITFORCOMMANDCAPTURE = new StateTimed(3000, []() {
        return TESTMQTTCONNECTION;
    });
    bootSequence.reset(new StateMachine({
        BOOTSEQUENCESTART,
        DELAYEDMQTTCONNECTION,
        TESTMQTTCONNECTION,
        CONNECTMQTT,
        PUBLISHONLINE,
        SUBSCRIBECOMMANDTOPIC,
        WAITFORCOMMANDCAPTURE
    }));

    startOTA();
    setupWiFi(properties);
    delay(50);

    EEPROM.begin(CRCEEProm::size(*settingsDTO.data()));

    SettingsDTOData data;
    bool loadedFromEEPROM = CRCEEProm::read(0, data);

    // Setup mqtt
    mqttClient.setServer(properties.get("mqtt_server").getCharPtr(), properties.get("mqtt_port").getLong());
    mqttClient.setCallback([](char* p_topic, byte * p_payload, uint16_t p_length) {
        char mqttReceiveBuffer[64];
        Serial.println("p_topic");

        if (p_length >= sizeof(mqttReceiveBuffer)) {
            DEBUG_PRINT(F("MQTT Message to long."));
            return;
        }

        memcpy(mqttReceiveBuffer, p_payload, p_length);
        mqttReceiveBuffer[p_length] = 0;
        handleCmd(p_topic, mqttReceiveBuffer);
    });

#ifdef DEMO_MODE
    temperatureSensor1.reset(mockedTemp1);
    temperatureSensor2.reset(mockedTemp2);
    ventilator1.reset(new MockedFan());
#else
    Adafruit_MAX31865* max31865 = new Adafruit_MAX31865(PIN_SPI_SCK);
    max31865->begin(MAX31865_2WIRE);
    temperatureSensor1.reset(new MAX31865sensor(max31865, RREF_OVEN, RNOMINAL_OVEN));

    Adafruit_MAX31855* max31855 = new Adafruit_MAX31855(PIN_SPI_SCK);
    max31855->begin();
    temperatureSensor2.reset(new MAX31855sensor(max31855));

    ventilator1.reset(new PWMVentilator(FAN1_PIN, 10.0));
#endif

    analogIn.reset(new AnalogIn(BUTTON_PIN, false, 50.0f, 50.0f, 220.0f, 0.1));
    analogKnobTemperatureSetPoint = 50.0;
    bbqController.reset(new BBQFanOnly(temperatureSensor1, ventilator1));

    if (loadedFromEEPROM) {
        BBQFanOnlyConfig config = bbqController->config();
        config.temp_alpha = data.temp_alpha;
        config.fan_low = data.fan_low;
        config.fan_medium = data.fan_medium;
        config.fan_high = data.fan_high;
        config.temp_error_low = data.temp_error_low;
        config.temp_error_medium = data.temp_error_medium;
        config.temp_error_hight = data.temp_error_hight;
        config.temp_change_fast = data.temp_change_fast;

        bbqController->config(config);
        bbqController->setPoint(data.setPoint);
    } else {
        bbqController->setPoint(analogIn->value());
    }

    bbqController->init();

    // Start UI
    ui.setTargetFPS(50);
    ui.setActiveSymbol(activeSymbol);
    ui.setInactiveSymbol(inactiveSymbol);
    ui.setIndicatorPosition(BOTTOM);
    ui.setIndicatorDirection(LEFT_RIGHT);
    ui.setFrameAnimation(SLIDE_UP);
    ui.setFrames(frames, 4);
    ui.setOverlays(overlays, 1);
    ui.init();

    display.flipScreenVertically();

    // Start boot sequence
    bootSequence->start();

    Serial.println(F("End Setup"));
    // Avoid running towards millis() when loop starts since we do effectPeriodStartMillis += EFFECT_PERIOD_CALLBACK;
    effectPeriodStartMillis = millis();
}

#define NUMBER_OF_SLOTS 12
void loop() {
    const uint32_t currentMillis = millis();
    int remainingTimeBudget = ui.update();

    if (remainingTimeBudget > 0 && currentMillis - effectPeriodStartMillis >= EFFECT_PERIOD_CALLBACK) {
        effectPeriodStartMillis += EFFECT_PERIOD_CALLBACK;
        transitionCounter++;
        uint8_t slot = 0;

        // Handle BBQ control
        if (transitionCounter % 5 == slot++) {
            bbqController->handle();
        }

        // Handle temperature reading
        if (transitionCounter % 5 == slot++) {
            temperatureSensor1->handle();
        }

        // once a second publish status to mqtt
        if (transitionCounter % 50 == slot++) {
            publishStatus();
        }

        if (transitionCounter % NUMBER_OF_SLOTS == slot++) {
            ArduinoOTA.handle();
        } else if (transitionCounter % NUMBER_OF_SLOTS == slot++) {
            float probeTemp = round(analogIn->value() * 2.0f) / 2.0f;

            // Only set new temperature when it changes more then half a degree
            if (fabs(probeTemp - analogKnobTemperatureSetPoint) > 0.5) {
                analogKnobTemperatureSetPoint = probeTemp;
                bbqController->setPoint(analogKnobTemperatureSetPoint);
            }
        } else if (transitionCounter % NUMBER_OF_SLOTS == slot++) {
            bootSequence->handle();
        } else if (transitionCounter % NUMBER_OF_SLOTS == slot++) {
            mqttClient.loop();
        } else if (transitionCounter % NUMBER_OF_SLOTS == slot++) {
            eepromSaveHandler.handle();
        } else if (transitionCounter % NUMBER_OF_SLOTS == slot++) {
            mqttSaveHandler.handle();
        } else if (transitionCounter % NUMBER_OF_SLOTS == slot++) {
            settingsDTO.reset();
        }

#if defined(ARILUX_DEBUG_TELNET)
        else if (transitionCounter % NUMBER_OF_SLOTS == slot++) {
        }

#endif
    }
}
