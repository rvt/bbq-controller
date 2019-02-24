/*
 */
#include <memory>
#include <cstring>
#include <vector>

#include "debug.h"

#include <ESP8266WiFi.h>  // https://github.com/esp8266/Arduino
#include <ESP8266mDNS.h>

#include <propertyutils.h>
#include <optparser.h>
#include <utils.h>
#include <icons.h>
#include <crceeprom.h>
#include <pwmventilator.h>
#include <onoffventilator.h>
#include <makestring.h>

// #include <spi.h> // Include for harware SPI
#include <Adafruit_MAX31855.h>
#include <max31855sensor.h>
#include <max31865sensor.h>
#include <PubSubClient.h> // https://github.com/knolleary/pubsubclient/releases/tag/v2.6

#include "ssd1306displaycontroller.h"
#include <ArduinoOTA.h>
#include <ESP_EEPROM.h>
#include <settings.h>
#include <analogin.h>
#include <digitalknob.h>
#include <Fuzzy.h>

#include "settingsdto.h"

#include <bbqfanonly.h>
#include <bbq.h>
#include <demo.h>
#include <statemachine.h>


// of transitions
volatile uint32_t counter50TimesSec = 1;

// Number calls per second we will be handling
#define FRAMES_PER_SECOND        50
#define EFFECT_PERIOD_CALLBACK   (1000 / FRAMES_PER_SECOND)

// Keep track when the last time we ran the effect state changes
volatile uint32_t effectPeriodStartMillis = 0;


typedef PropertyValue PV ;
Properties properties;

// Display System
SSD1306DisplayController ssd1306displayController(WIRE_SDA, WIRE_SCL);

// bbqCOntroller, sensors and ventilators
std::unique_ptr<BBQFanOnly> bbqController(nullptr);
std::shared_ptr<TemperatureSensor> temperatureSensor1(nullptr);
std::shared_ptr<TemperatureSensor> temperatureSensor2(nullptr);
std::shared_ptr<Ventilator> ventilator1(nullptr);

// START: For demo/test mode
MockedTemperature* mockedTemp1 = new MockedTemperature(20.0);
MockedTemperature* mockedTemp2 = new MockedTemperature(30.0);
// END: For demo/test mode


std::shared_ptr<AnalogIn> analogIn = std::make_shared<AnalogIn>(0.2f);
DigitalKnob digitalKnob(BUTTON_PIN, true, 110);

// Settings
std::unique_ptr<SettingsDTO> settingsDTO(nullptr);

// CRC value of last update to MQTT
uint16_t lastUpdateCRC = 0;

// Eeprom storage
// wait 500ms after last commit, then commit no more often than every 30s
Settings eepromSaveHandler(
    500,
    10000,
[]() {
    CRCEEProm::write(0, *settingsDTO->data());
    EEPROM.commit();
},
[]() {
    return settingsDTO->modified();
}
);

// MQTT Storage
// mqtt updates as quickly as possible with a maximum frequence of MQTT_STATE_UPDATE_DELAY
void publishToMQTT(const char* topic, const char* payload);
Settings mqttSaveHandler(
    1,
    MQTT_STATE_UPDATE_DELAY,
[]() {

    std::string configString = settingsDTO->getConfigString();

    if (configString.size() > 0) {
        Serial.println(configString.c_str());
        Serial.println(properties.get("mqttConfigStateTopic").getCharPtr());
        publishToMQTT(properties.get("mqttConfigStateTopic").getCharPtr(), configString.c_str());
    }

},
[]() {
    return settingsDTO->modified();
}
);

// State machine states and configurations
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

            // Copy setpoint value
            if (strcmp(values.key(), "sp") == 0) {
                temperature = values.asFloat();
            }

            // Fan On/Off controller duty cycle
            if (strcmp(values.key(), "ood") == 0) {
                settingsDTO->data()->on_off_fan_duty_cycle = between(values.asLong(), (int32_t)5000, (int32_t)120000);
                ventilator1.reset(new OnOffVentilator(FAN1_PIN, settingsDTO->data()->on_off_fan_duty_cycle));
            }

            // Lid open fan speed
            if (strcmp(values.key(), "lof") == 0) {
                config.fan_speed_lid_open = between((int8_t)values.asInt(), (int8_t) -1, (int8_t)100);
            }

            // Copy minimum fan1 PWM speed in %
            if (strcmp(values.key(), "fs1") == 0) {
                settingsDTO->data()->fan_startPwm1 = values.asInt();
                // Tech Debth? Can we get away with static_pointer_cast without Ventilator knowing about any setPwmStart?
                std::static_pointer_cast<PWMVentilator>(ventilator1)->setPwmStart(values.asInt());
            }

            // Fan 1 override ( we don´t want this as an settings so if we loose MQTT connection we can always unplug)
            if (strcmp(values.key(), "f1o") == 0) {
                ventilator1->speedOverride(between(values.asFloat(), -1.0f, 100.0f));
            }

            config.fan_low = getConfigArray("fl1", values.key(), values.asChar(), config.fan_low);
            config.fan_medium = getConfigArray("fm1", values.key(), values.asChar(), config.fan_medium);
            config.fan_high = getConfigArray("fh1", values.key(), values.asChar(), config.fan_high);

            config.temp_error_low = getConfigArray("tel", values.key(), values.asChar(), config.temp_error_low);
            config.temp_error_medium = getConfigArray("tem", values.key(), values.asChar(), config.temp_error_medium);
            config.temp_error_hight = getConfigArray("teh", values.key(), values.asChar(), config.temp_error_hight);

            config.temp_change_fast = getConfigArray("tcf", values.key(), values.asChar(), config.temp_change_fast);
        });

        if (temperature > 1.0f) {
            settingsDTO->data()->setPoint = between(temperature, 90.0f, 240.0f);
            bbqController->setPoint(settingsDTO->data()->setPoint);
        }

        // Copy to settings
        settingsDTO->data()->lid_open_fan_speed = config.fan_speed_lid_open;
        settingsDTO->data()->fan_low = config.fan_low;
        settingsDTO->data()->fan_medium = config.fan_medium;
        settingsDTO->data()->fan_high = config.fan_high;
        settingsDTO->data()->temp_error_low = config.temp_error_low;
        settingsDTO->data()->temp_error_medium = config.temp_error_medium;
        settingsDTO->data()->temp_error_hight = config.temp_error_hight;
        settingsDTO->data()->temp_change_fast = config.temp_change_fast;

        // Update the bbqController with new values
        bbqController->config(config);
        bbqController->init();
    }

    if (strstr(topicPos, "reset") != nullptr) {
        OptParser::get(p_payload, [](OptValue v) {
            if (strcmp(v.key(), "1") == 0) {
                Serial.println("reset SettingsDTOData");
                SettingsDTOData d;
                CRCEEProm::write(0, d);
                EEPROM.commit();
                // Restart didn´t work yet
            }
        });
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
}

///////////////////////////////////////////////////////////////////////////
//  SETUP() AND LOOP()
///////////////////////////////////////////////////////////////////////////

void setup() {
    //********** CHANGE PIN FUNCTION  TO GPIO **********
    //https://www.esp8266.com/wiki/doku.php?id=esp8266_gpio_pin_allocations
    //GPIO 1 (TX) swap the pin to a GPIO.
    pinMode(1, FUNCTION_3);
    //GPIO 3 (RX) swap the pin to a GPIO.
    pinMode(3, FUNCTION_3);
    //**************************************************

    // Enable serial port
    Serial.begin(115200);
    delay(50);
    Serial.print(F("Hostname: "));
    // setup Strings
    loadConfiguration(properties);
    Serial.println(properties.get("mqttClientID").getCharPtr());

    State* BOOTSEQUENCESTART;
    State* DELAYEDMQTTCONNECTION;
    State* TESTMQTTCONNECTION;
    State* CONNECTMQTT;
    State* PUBLISHONLINE;
    State* SUBSCRIBECOMMANDTOPIC;
    State* WAITFORCOMMANDCAPTURE;

    BOOTSEQUENCESTART = new State([]() {
        return 2;
    });
    DELAYEDMQTTCONNECTION = new StateTimed(1500, []() {
        return 2;
    });
    TESTMQTTCONNECTION = new State([]() {
        if (mqttClient.connected())  {
            if (WiFi.status() != WL_CONNECTED) {
                mqttClient.disconnect();
            }

            return 1;
        }

        return 3;
    });
    CONNECTMQTT = new State([]() {
        if (mqttClient.connect(
                properties.get("mqttClientID").getCharPtr(),
                MQTT_USER,
                MQTT_PASS,
                properties.get("mqttLastWillTopic").getCharPtr(),
                0, 1, MQTT_LASTWILL_OFFLINE)) {
            return 4;
        }

        DEBUG_PRINTLN(F("ERROR: The connection to the MQTT broker failed"));
        DEBUG_PRINT(F("Username: "));
        DEBUG_PRINTLN(MQTT_USER);
        DEBUG_PRINT(F("Broker: "));
        DEBUG_PRINTLN(MQTT_SERVER);
        return 1;
    });
    PUBLISHONLINE = new State([]() {
        publishToMQTT(
            properties.get("mqttLastWillTopic").getCharPtr(),
            MQTT_LASTWILL_ONLINE);
        return 5;
    });
    SUBSCRIBECOMMANDTOPIC = new State([]() {
        if (mqttClient.subscribe(properties.get("mqttSubscriberTopic").getCharPtr(), 0)) {
            Serial.println(properties.get("mqttSubscriberTopic").getCharPtr());
            return 6;
        }

        DEBUG_PRINT(F("ERROR: Failed to connect to topic : "));
        mqttClient.disconnect();
        return 1;
    });
    WAITFORCOMMANDCAPTURE = new StateTimed(3000, []() {
        return 2;
    });
    bootSequence.reset(new StateMachine({
        BOOTSEQUENCESTART, // 0
        DELAYEDMQTTCONNECTION,// 1
        TESTMQTTCONNECTION, // 2
        CONNECTMQTT, // 3
        PUBLISHONLINE, // 4
        SUBSCRIBECOMMANDTOPIC, // 5
        WAITFORCOMMANDCAPTURE // 6
    }));

    startOTA();
    setupWiFi(properties);
    delay(50);

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

    EEPROM.begin(CRCEEProm::size(*settingsDTO->data()));
    SettingsDTOData data;
    bool loadedFromEEPROM = CRCEEProm::read(0, data);

    if (loadedFromEEPROM) {
        settingsDTO.reset(new SettingsDTO(data));
    } else {
        settingsDTO.reset(new SettingsDTO());
    }

#ifdef DEMO_MODE
    temperatureSensor1.reset(mockedTemp1);
    temperatureSensor2.reset(mockedTemp2);
    ventilator1.reset(new MockedFan());
#else
    // Sensor 1 is generally used for the temperature of the bit
    auto sensor1 = new MAX31865sensor(SPI_MAX31865_CS_PIN, SPI_SDI_PIN, SPI_SDO_PIN, SPI_CLK_PIN, RNOMINAL_OVEN, RREF_OVEN);
    sensor1->begin(MAX31865_3WIRE);
    temperatureSensor1.reset(sensor1);

    // Sensor 2 is generally used to measure the temperature of the pit itself
    auto sensor2 = new Adafruit_MAX31855(SPI_CLK_PIN, SPI_MAX31855_CS_PIN, SPI_SDI_PIN);
    sensor2->begin();
    temperatureSensor2.reset(new MAX31855sensor(sensor2));

    #if PWM_FAN == 1
    ventilator1.reset(new PWMVentilator(FAN1_PIN, settingsDTO->data()->fan_startPwm1));
    #elif ON_OFF_FAN == 1
    ventilator1.reset(new OnOffVentilator(FAN1_PIN, settingsDTO->data()->on_off_fan_duty_cycle));
    #else
    #error Should pick PWM_FAN or ON_OFF_FAN
    #endif
#endif

    bbqController.reset(new BBQFanOnly(temperatureSensor1, ventilator1));
    BBQFanOnlyConfig config = bbqController->config();
    config.fan_low = settingsDTO->data()->fan_low;
    config.fan_medium = settingsDTO->data()->fan_medium;
    config.fan_high = settingsDTO->data()->fan_high;
    config.temp_error_low = settingsDTO->data()->temp_error_low;
    config.temp_error_medium = settingsDTO->data()->temp_error_medium;
    config.temp_error_hight = settingsDTO->data()->temp_error_hight;
    config.temp_change_fast = settingsDTO->data()->temp_change_fast;
    bbqController->config(config);
    bbqController->setPoint(settingsDTO->data()->setPoint);

    bbqController->init();

    // Start boot sequence
    bootSequence->start();

    // Init display controller
    ssd1306displayController.init();

    Serial.println(F("End Setup"));
    // Avoid running towards millis() when loop starts since we do effectPeriodStartMillis += EFFECT_PERIOD_CALLBACK;
    effectPeriodStartMillis = millis();
}

#define NUMBER_OF_SLOTS 10
void loop() {
    const uint32_t currentMillis = millis();
    int remainingTimeBudget = ssd1306displayController.handle();

    if (remainingTimeBudget > 0 && currentMillis - effectPeriodStartMillis >= EFFECT_PERIOD_CALLBACK) {
        effectPeriodStartMillis += EFFECT_PERIOD_CALLBACK;
        counter50TimesSec++;

        // DigitalKnob (the button) must be handled at 50 times/sec to correct handle presses and double presses
        digitalKnob.handle();
        // Handle analog input
        analogIn -> handle();

        // Handle BBQ inputs once every 5 seconds
        if (counter50TimesSec % 50 == 0) {
            bbqController -> handle();
        }

        // once a second publish status to mqtt (if there are changes)
        if (counter50TimesSec % 50 == 0) {
            publishStatus();
        }

        ventilator1->handle();

        // Maintenance stuff
        uint8_t slot50 = 0;

        if (counter50TimesSec % NUMBER_OF_SLOTS == slot50++) {
            ArduinoOTA.handle();
        } else if (counter50TimesSec % NUMBER_OF_SLOTS == slot50++) {
            bootSequence->handle();
        } else if (counter50TimesSec % NUMBER_OF_SLOTS == slot50++) {
            mqttClient.loop();
        } else if (counter50TimesSec % NUMBER_OF_SLOTS == slot50++) {
            eepromSaveHandler.handle();
        } else if (counter50TimesSec % NUMBER_OF_SLOTS == slot50++) {
            mqttSaveHandler.handle();
        } else if (counter50TimesSec % NUMBER_OF_SLOTS == slot50++) {
            settingsDTO->reset();
        } else if (counter50TimesSec % NUMBER_OF_SLOTS == slot50++) {
            temperatureSensor1->handle();
        } else if (counter50TimesSec % NUMBER_OF_SLOTS == slot50++) {
            temperatureSensor2->handle();
        }


    }
}
