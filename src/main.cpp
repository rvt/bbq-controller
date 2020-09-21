/*
 */
#include <memory>
#include <cstring>
#include <vector>

#include "makestring.h"

#include "displayController.h"
extern "C" {
#include <crc16.h>
}

#if defined(ESP32)
#define FileSystemFS SPIFFS
#define FileSystemFSBegin() SPIFFS.begin(true)
#include <WiFi.h>
#include <esp_wifi.h>
#include "rotaryencoder.h"

#define WIFI_getChipId() (uint32_t)ESP.getEfuseMac()

#include <ESPmDNS.h>
#include <SPIFFS.h>

#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <LittleFS.h>
#include <stefanspwmventilator.h>

extern "C" {
#include "user_interface.h"
}
#include <ESP8266WebServer.h>

#define WIFI_getChipId() ESP.getChipId()

#define FileSystemFS LittleFS
#define FileSystemFSBegin() LittleFS.begin()
#endif


#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include "ssd1306displaycontroller.h"
#include "TTGO_T_DisplayController.h"
#include <propertyutils.h>
#include <optparser.h>
#include <utils.h>
#include <icons.h>
#include <pwmventilator.h>
#include <onoffventilator.h>
#include <settings.h>

#include <SPI.h> // Include for harware SPI
#include <Wire.h>
#include <max31855sensor.h>
#include <max31865sensor.h>
#include <NTCSensor.h>
#include <PubSubClient.h> // https://github.com/knolleary/pubsubclient/releases/tag/v2.6

#include <config.h>
#include <analogin.h>
#include <digitalknob.h>
#include <Fuzzy.h>

#include <bbqfanonly.h>
#include <bbq.h>
#include <demo.h>
#include <statemachine.h>
#include <StreamUtils.h>

typedef PropertyValue PV ;

// of transitions
uint32_t counter50TimesSec = 1;

// Number calls per second we will be handling
#define FRAMES_PER_SECOND        50
#define EFFECT_PERIOD_CALLBACK   (1000 / FRAMES_PER_SECOND)
constexpr uint8_t LINE_BUFFER_SIZE = 64;

// Keep track when the last time we ran the effect state changes
uint32_t effectPeriodStartMillis = 0;

// Display System
#if defined(TTG_T_DISPLAY)
DisplayController* displayController = new TTGO_T_DisplayController();
#elif defined(GEEKKCREIT_OLED)
DisplayController* displayController = new SSD1306DisplayController(WIRE_SDA, WIRE_SCL);
#else
#error Must have a displaydriver use TTG_T_DISPLAY or GEEKKCREIT_OLED
#endif

// bbqCOntroller, sensors and ventilators
std::unique_ptr<BBQFanOnly> bbqController(nullptr);
std::shared_ptr<TemperatureSensor> temperatureSensor1(nullptr);
std::shared_ptr<TemperatureSensor> temperatureSensor2(nullptr);
std::shared_ptr<Ventilator> ventilator1(nullptr);

// START: For demo/test mode
MockedTemperature* mockedTemp1 = new MockedTemperature(20.0);
MockedTemperature* mockedTemp2 = new MockedTemperature(30.0);
// END: For demo/test mode

// WiFI Manager
WiFiManager wm;


DigitalKnob digitalKnob(BUTTON_PIN, true, 110);
#if defined(GEEKKCREIT_OLED)
// Analog and digital inputs
std::shared_ptr<AnalogIn> analogIn = std::make_shared<AnalogIn>(0.2f);
#pragma message "Using analog potentiometer for menu"
#elif defined(TTG_T_DISPLAY)
DigitalKnob rotary1(ROTARY_PIN1, true, 1000);
DigitalKnob rotary2(ROTARY_PIN2, true, 1000);
#pragma message "Using rotary encoder for menu"
#endif
// Stores information about the BBQ controller (PID values, fuzzy loggic values etc, mqtt)
Properties controllerConfig;
bool controllerConfigModified = false;
// Stores information about the current temperature settings
Properties bbqConfig;
bool bbqConfigModified = false;

// CRC value of last update to MQTT
uint16_t lastMeasurementCRC = 0;
uint32_t shouldRestart = 0;        // Indicate that a service requested an restart. Set to millies() of current time and it will restart 5000ms later

bool hasMqttConfigured = false;
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

// State machine states and configurations
std::unique_ptr<StateMachine> bootSequence(nullptr);

#define MQTT_SERVER_LENGTH 40
#define MQTT_PORT_LENGTH 5
#define MQTT_USERNAME_LENGTH 18
#define MQTT_PASSWORD_LENGTH 18
WiFiManagerParameter wm_mqtt_server("server", "mqtt server", "", MQTT_SERVER_LENGTH);
WiFiManagerParameter wm_mqtt_port("port", "mqtt port", "", MQTT_PORT_LENGTH);
WiFiManagerParameter wm_mqtt_user("user", "mqtt username", "", MQTT_USERNAME_LENGTH);

const char _customHtml_hidden[] = "type=\"password\"";
WiFiManagerParameter wm_mqtt_password("input", "mqtt password", "", MQTT_PASSWORD_LENGTH, _customHtml_hidden, WFM_LABEL_AFTER);


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

bool saveConfig(const char* filename, Properties& properties);

Settings saveBBQConfigHandler(
    500,
    10000,
[]() {
    saveConfig(BBQ_CONFIG_FILENAME, bbqConfig);
    bbqConfigModified = false;
},
[]() {
    return bbqConfigModified;
}
);

bool loadConfig(const char* filename, Properties& properties) {
    bool ret = false;

    if (FileSystemFSBegin()) {
        Serial.println("mounted file system");

        if (FileSystemFS.exists(filename)) {
            //file exists, reading and loading
            File configFile = FileSystemFS.open(filename, "r");

            if (configFile) {
                Serial.print(F("Loading config : "));
                Serial.println(filename);
                deserializeProperties<LINE_BUFFER_SIZE>(configFile, properties);
                // serializeProperties<LINE_BUFFER_SIZE>(Serial, properties);
            }

            configFile.close();
        } else {
            Serial.print(F("File not found: "));
            Serial.println(filename);
        }

        // FileSystemFS.end();
    } else {
        Serial.print(F("Failed to begin FileSystemFS"));
    }

    return ret;
}


/**
 * Store custom oarameter configuration in FileSystemFS
 */
bool saveConfig(const char* filename, Properties& properties) {
    bool ret = false;

    if (FileSystemFSBegin()) {
        FileSystemFS.remove(filename);
        File configFile = FileSystemFS.open(filename, "w");

        if (configFile) {
            Serial.print(F("Saving config : "));
            Serial.println(filename);
            serializeProperties<LINE_BUFFER_SIZE>(configFile, properties);
            // serializeProperties<LINE_BUFFER_SIZE>(Serial, properties);
            ret = true;
        } else {
            Serial.print(F("Failed to write file"));
            Serial.println(filename);
        }

        configFile.close();
        //    FileSystemFS.end();
    }

    return ret;
}

///////////////////////////////////////////////////////////////////////////
//  MQTT
///////////////////////////////////////////////////////////////////////////
/*
* Publish current status
* to = temperature oven
* sp = set Point
* f1 = Speed of fan 1
* lo = Lid open alert
* lc = Low charcoal alert
* ft = Type of ventilator
*/
void publishToMQTT(const char* topic, const char* payload);

void publishStatusToMqtt() {
    char* format;
    char* buffer;

    // Can we do better than this?
    if (controllerConfig.get("statusJson")) {
        static char f[] = "{\"to\":%.2f,\"t2\":%.2f,\"sp\":%.2f,\"f1\":%.2f,\"lo\":%i,\"lc\":%i,\"f1o\":%.1f,\"ft\":%i}";
        static char b[sizeof(f) + 3 * 8 + 10]; // 2 bytes per extra item + 10 extra
        format = f;
        buffer = b;
    } else {
        static char f[] = "to=%.2f t2=%.2f sp=%.2f f1=%.2f lo=%i lc=%i f1o=%.1f ft=%i";
        static char b[sizeof(f) + 3 * 8 + 10]; // 2 bytes per extra item + 10 extra
        format = f;
        buffer = b;
    }

    sprintf(buffer, format,
            temperatureSensor1->get(),
            temperatureSensor2->get(),
            bbqController->setPoint(),
            ventilator1->speed(),
            false, // bbqController->lidOpen(),
            bbqController->lowCharcoal(),
            ventilator1->speedOverride(),
            (int16_t)controllerConfig.get("fan1Type")
           );

    // Quick hack to only update when data actually changed
    uint16_t thisCrc = crc16((uint8_t*)buffer, std::strlen(buffer));

    if (thisCrc != lastMeasurementCRC) {
        publishToMQTT("status", buffer);
        lastMeasurementCRC = thisCrc;
    }
}

/**
 * Publish a message to mqtt
 */
void publishToMQTT(const char* topic, const char* payload) {
    if (!mqttClient.connected()) {
        return;
    }

    char buffer[65];
    const char* mqttBaseTopic = controllerConfig.get("mqttBaseTopic");
    snprintf(buffer, sizeof(buffer), "%s/%s", mqttBaseTopic, topic);

    if (mqttClient.publish(buffer, payload, true)) {
    } else {
        Serial.println(F("Failed to publish"));
    }
}

/////////////////////////////////////////////////////////////////////////////////////

/**
 * Handle incomming MQTT requests
 */
void setupIOHardware();
void handleCmd(const char* topic, const char* p_payload) {
    auto topicPos = topic + strlen(controllerConfig.get("mqttBaseTopic"));
    Serial.print(F("Handle command : "));
    Serial.print(topicPos);
    Serial.print(F(" : "));
    Serial.println(p_payload);

    // Look for a temperature setPoint topic
    char payloadBuffer[32];
    strncpy(payloadBuffer, p_payload, sizeof(payloadBuffer));

    if (std::strstr(topicPos, "/config") != nullptr) {
        BBQFanOnlyConfig config = bbqController->config();
        float temperature = 0;

        OptParser::get(payloadBuffer, [&config, &temperature](OptValue values) {

            // Copy setpoint value
            if (std::strcmp(values.key(), "sp") == 0) {
                temperature = values;
            }

            // Fan On/Off controller duty cycle
            if (std::strcmp(values.key(), "ood") == 0) {
                int32_t v = between((int32_t)values, (int32_t)5000, (int32_t)120000);
                controllerConfig.put("fanOnOffDutyCycle", PV(v));
                controllerConfigModified = true;
            }

            if (std::strcmp(values.key(), "ft") == 0) {
                int32_t v = between((int32_t)values, 0, 1);
                controllerConfig.put("fan1Type", PV(v));
                controllerConfigModified = true;
                setupIOHardware();
            }

            // Lid open fan speed
            if (strcmp(values.key(), "lof") == 0) {
                config.fan_speed_lid_open = between((int8_t)values, (int8_t) -1, (int8_t)100);
            }

            // Fan 1 override ( we don´t want this as an settings so if we loose MQTT connection we can always unplug)
            if (strcmp(values.key(), "f1o") == 0) {
                ventilator1->speedOverride(values);
            }

            config.fan_lower = getConfigArray("flo", values.key(), values, config.fan_lower);
            config.fan_steady = getConfigArray("fst", values.key(), values, config.fan_steady);
            config.fan_higher = getConfigArray("fhi", values.key(), values, config.fan_higher);

            config.temp_error_low = getConfigArray("tel", values.key(), values, config.temp_error_low);
            config.temp_error_medium = getConfigArray("tem", values.key(), values, config.temp_error_medium);
            config.temp_error_hight = getConfigArray("teh", values.key(), values, config.temp_error_hight);

            config.temp_change_slow = getConfigArray("tcs", values.key(), values, config.temp_change_slow);
            config.temp_change_medium = getConfigArray("tcm", values.key(), values, config.temp_change_medium);
            config.temp_change_fast = getConfigArray("tcf", values.key(), values, config.temp_change_fast);
        });

        if (temperature > 1.0f) {
            bbqController->setPoint(temperature);
            bbqConfig.put("setPoint", PV(temperature));
            bbqConfigModified = true;
        }

        Serial.println("Config received");
        // Update the bbqController with new values
        bbqController->config(config);
        bbqController->init();
    }

    if (std::strstr(topicPos, "/controllerConfig") != nullptr) {
        Serial.println("controllerConfig received");
        StringStream stream;
        stream.print(payloadBuffer);
        deserializeProperties<LINE_BUFFER_SIZE>(stream, controllerConfig);
        controllerConfigModified = true;
        setupIOHardware();
    }

    if (strstr(topicPos, "/reset") != nullptr) {
        if (strcmp(payloadBuffer, "1") == 0) {
            shouldRestart = millis();
        }
    }

    // Dummy data topic during testing
    // With ot we can simulate a oven temperature
    // BBQ/xxxxxx/dummy
#ifdef DEMO_MODE

    if (strstr(topicPos, TEMPERATURE_DUMMY_TOPIC) != nullptr) {
        OptParser::get(p_payload, [](OptValue v) {
            if (strcmp(v.key(), "t1") == 0) {
                mockedTemp1->set(v);
            }

            if (strcmp(v.key(), "t2") == 0) {
                mockedTemp2->set(v);
            }
        });
    }

#endif
}

/**
 * Initialise MQTT and variables
 */
void setupMQTT() {

    mqttClient.setCallback([](char* p_topic, byte * p_payload, uint16_t p_length) {
        char mqttReceiveBuffer[64];
        //Serial.println(p_topic);

        if (p_length >= sizeof(mqttReceiveBuffer)) {
            return;
        }

        memcpy(mqttReceiveBuffer, p_payload, p_length);
        mqttReceiveBuffer[p_length] = 0;
        handleCmd(p_topic, mqttReceiveBuffer);
    });

}

///////////////////////////////////////////////////////////////////////////
//  IOHardware
///////////////////////////////////////////////////////////////////////////

Ventilator* createVentilator(uint8_t num) {
    // Get the old fan so we can copy it´s settings
    std::shared_ptr<Ventilator> oldFan = ventilator1;

    Ventilator* ventilator;
    if (num == 0) {
        //ventilator1.reset(new StefansPWMVentilator(FAN1_PIN, (int16_t)controllerConfig.get("fan1Start")));
        ventilator = new PWMVentilator(FAN1_PIN, (int16_t)controllerConfig.get("fan1Start"));
    } else {
        ventilator = new OnOffVentilator(FAN1_PIN, (int16_t)controllerConfig.get("fanOnOffDutyCycle"));
    }

    if (oldFan.get() != nullptr) {
        ventilator->speedOverride(oldFan->speedOverride());
        ventilator->setOn(oldFan->isOn());
    }

    return ventilator;
}

void getRc1c2c3(const char* buffer, float* r, float* c1, float* c2, float* c3, float* o) {
    char buf[64];
    strncpy(buf, buffer, strlen(buf));
    OptParser::get(buf, ',', [&r, &c1, &c2, &c3, &o](OptValue value) {
        if (std::strcmp(value.key(), "r") == 0) {
            *r = (float)value;
        } else if (std::strcmp(value.key(), "c1") == 0) {
            *c1 = (float)value;
        } else if (std::strcmp(value.key(), "c2") == 0) {
            *c2 = (float)value;
        } else if (std::strcmp(value.key(), "c3") == 0) {
            *c3 = (float)value;
        } else if (std::strcmp(value.key(), "o") == 0) {
            *o = (float)value;
        }
    });
}

TemperatureSensor* createTemperatureSensor(uint8_t num) {
    switch (num) {
        case 0: {
            MAX31865sensor* sensor1 = new MAX31865sensor(SPI_MAX31865_CS_PIN, SPI_SDI_PIN, SPI_SDO_PIN, SPI_CLK_PIN, RNOMINAL_OVEN, RREF_OVEN);
            sensor1->begin(MAX31865_3WIRE);
            return sensor1;
            break;
        }
        case 1: {
            Adafruit_MAX31855* sensor2 = new Adafruit_MAX31855(SPI_CLK_PIN, SPI_MAX31855_CS_PIN, SPI_SDO_PIN);
            sensor2->begin();
            return new MAX31855sensor(sensor2);
            break;
        }
        case 2: {
            char parameter[16];
            float r,c1,c2,c3,offset;
            snprintf(parameter, sizeof(parameter), "NTC%dStein", num);
            getRc1c2c3(controllerConfig.get(parameter), &r, &c1, &c2, &c3, &offset);
            snprintf(parameter, sizeof(parameter), "NTC%dPin", num);
            uint8_t pin = (int16_t)controllerConfig.get(parameter);
            return new NTCSensor(pin, offset, r, c1, c2, c3);  
            break;
        }
    }
    return nullptr;
}

void setupIOHardware() {
    temperatureSensor2.reset(createTemperatureSensor((int16_t)controllerConfig.get("sensor2Type")));
    temperatureSensor1.reset(createTemperatureSensor((int16_t)controllerConfig.get("sensor1Type")));
    ventilator1.reset(createVentilator((int16_t)controllerConfig.get("fan1Type")));
    
    digitalKnob.init();
#if defined(TTG_T_DISPLAY)
    rotary1.init();
    rotary2.init();
#endif

    bbqController.reset(new BBQFanOnly(temperatureSensor1, ventilator1));
    bbqController->init();
    bbqController->setPoint(bbqConfig.get("setPoint"));
}

///////////////////////////////////////////////////////////////////////////
//  BBQT Controller
///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////
//  WiFi
///////////////////////////////////////////////////////////////////////////


void serverOnlineCallback() {
    /*     wm.server->on(JBONE_URI, []() {
            wm.server->sendHeader("Content-Encoding", "gzip");
            wm.server->setContentLength(jscript_js_gz_len);
            wm.server->send(200, "application/javascript", "");
            wm.server->sendContent_P((char*)jscript_js_gz, jscript_js_gz_len);
        });

        wm.server->on(TRACK_PEEK_URI, []() {
            char payloadBuffer[128];
            sprintf(payloadBuffer, "{\"yaw\":%.2f,\"pitch\":%.2f,\"roll\":%.2f,\"x\":%.2f,\"y\":%.2f,\"z\":%.2f}",
                    last_measurement.yaw * 180 / M_PI,
                    last_measurement.pitch * 180 / M_PI,
                    last_measurement.roll * 180 / M_PI,
                    last_measurement.x,
                    last_measurement.y,
                    last_measurement.z
                   );

            wm.server->setContentLength(std::strlen(payloadBuffer));
            wm.server->send(200, F("application/javascript"), payloadBuffer);
        });

        wm.server->on(STORE_CALIBRATION_URI, []() {
            if (hwTrack->isReady()) {

                if (!json.containsKey(hwTrack->name())) {
                    json.createNestedObject(hwTrack->name());
                }
                JsonObject config = json[hwTrack->name()].as<JsonObject>();
                hwTrack->calibrate(config);
                serializeJsonPretty(config, Serial);

                shouldSaveConfig = true;

                // Send result back
                wm.server->setContentLength(measureJson(config));
                wm.server->send(200, F("application/javascript"), "");
                WiFiClient client = wm.server->client();
                serializeJson(config, client);

                // Request restart
                shouldRestart = millis();
            } else {
                // We would like to send a 503 but tje JS framework doesn´t give us the body
                wm.server->send(200, F("application/json"), F("{\"status\":\"error\", \"message\":\"MPU is not ready, please check hardware.\"}"));
            }
        }); */
}

/**
 * Setup statemachine that will handle reconnection to mqtt after WIFI drops
 */
void setupWIFIReconnectManager() {
    // Statemachine to handle (re)connection to MQTT
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
        hasMqttConfigured =
            controllerConfig.contains("mqttServer") &&
            std::strlen((const char*)controllerConfig.get("mqttServer")) > 0;

        if (!hasMqttConfigured) {
            return 1;
        }

        return 2;
    });
    TESTMQTTCONNECTION = new State([]() {
        if (mqttClient.connected())  {
            if (WiFi.status() != WL_CONNECTED) {
                mqttClient.disconnect();
            }

            return 1;
        }

        // For some reason the access point active, so we disable it explicitly
        // FOR ESP32 we will keep on this state untill WIFI is connected
        if (WiFi.status() == WL_CONNECTED) {
            WiFi.mode(WIFI_STA);
        } else {
            return 2;
        }

        return 3;
    });
    CONNECTMQTT = new State([]() {
        mqttClient.setServer(
            controllerConfig.get("mqttServer"),
            (int16_t)controllerConfig.get("mqttPort")
        );

        if (mqttClient.connect(
                controllerConfig.get("mqttClientID"),
                controllerConfig.get("mqttUsername"),
                controllerConfig.get("mqttPassword"),
                controllerConfig.get("mqttLastWillTopic"),
                0,
                1,
                MQTT_LASTWILL_OFFLINE)
           ) {
            return 4;
        }

        return 1;
    });
    PUBLISHONLINE = new State([]() {
        publishToMQTT(
            MQTT_LASTWILL_TOPIC,
            MQTT_LASTWILL_ONLINE);
        return 5;
    });
    SUBSCRIBECOMMANDTOPIC = new State([]() {
        char mqttSubscriberTopic[32];
        strncpy(mqttSubscriberTopic, controllerConfig.get("mqttBaseTopic"), sizeof(mqttSubscriberTopic));
        strncat(mqttSubscriberTopic, "/+", sizeof(mqttSubscriberTopic));

        if (mqttClient.subscribe(mqttSubscriberTopic, 0)) {
            return 6;
        }

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
    bootSequence->start();
}

///////////////////////////////////////////////////////////////////////////
//  Webserver/WIFIManager
///////////////////////////////////////////////////////////////////////////
void saveParamCallback() {
    Serial.println("[CALLBACK] saveParamCallback fired");

    if (std::strlen(wm_mqtt_server.getValue()) > 0) {
        controllerConfig.put("mqttServer", PV(wm_mqtt_server.getValue()));
        controllerConfig.put("mqttPort", PV(std::atoi(wm_mqtt_port.getValue())));
        controllerConfig.put("mqttUsername", PV(wm_mqtt_user.getValue()));
        controllerConfig.put("mqttPassword", PV(wm_mqtt_password.getValue()));
        controllerConfigModified = true;
        // Redirect from MQTT so on the next reconnect we pickup new values
        mqttClient.disconnect();
        // Send redirect back to param page
        wm.server->sendHeader(F("Location"), F("/param?"), true);
        wm.server->send(302, FPSTR(HTTP_HEAD_CT2), "");   // Empty content inhibits Content-length header so we have to close the socket ourselves.
        wm.server->client().stop();
    }
}

/**
 * Setup the wifimanager and configuration page
 */
void setupWifiManager() {
    char port[6];
    snprintf(port, sizeof(port), "%d", (int16_t)controllerConfig.get("mqttPort"));
    wm_mqtt_port.setValue(port, MQTT_PORT_LENGTH);
    wm_mqtt_password.setValue(controllerConfig.get("mqttPassword"), MQTT_PASSWORD_LENGTH);
    wm_mqtt_user.setValue(controllerConfig.get("mqttUsername"), MQTT_USERNAME_LENGTH);
    wm_mqtt_server.setValue(controllerConfig.get("mqttServer"), MQTT_SERVER_LENGTH);

    wm.addParameter(&wm_mqtt_server);
    wm.addParameter(&wm_mqtt_port);
    wm.addParameter(&wm_mqtt_user);
    wm.addParameter(&wm_mqtt_password);

    /////////////////
    // set country
    wm.setClass("invert");
    wm.setCountry("US"); // setting wifi country seems to improve OSX soft ap connectivity, may help others as well

    // Set configuration portal
    wm.setShowStaticFields(false);
    wm.setConfigPortalBlocking(false); // Must be blocking or else AP stays active
    wm.setDebugOutput(false);
    wm.setWebServerCallback(serverOnlineCallback);
    wm.setSaveParamsCallback(saveParamCallback);
    wm.setHostname(controllerConfig.get("mqttClientID"));
    std::vector<const char*> menu = {"wifi", "wifinoscan", "info", "param", "sep", "erase", "restart"};
    wm.setMenu(menu);

    wm.startWebPortal();
    wm.autoConnect(controllerConfig.get("mqttClientID"));
#if defined(ESP8266)
    WiFi.setSleepMode(WIFI_NONE_SLEEP);
    MDNS.begin(controllerConfig.get("mqttClientID"));
    MDNS.addService(0, "http", "tcp", 80);
#endif
}

///////////////////////////////////////////////////////////////////////////
//  SETUP and LOOP
///////////////////////////////////////////////////////////////////////////

void setupDefaults() {
    char chipHexBuffer[9];
    snprintf(chipHexBuffer, sizeof(chipHexBuffer), "%08X", WIFI_getChipId());

    char mqttClientID[16];
    snprintf(mqttClientID, sizeof(mqttClientID), "BBQ_%s", chipHexBuffer);

    char mqttBaseTopic[16];
    snprintf(mqttBaseTopic, sizeof(mqttBaseTopic), "BBQ/%s", chipHexBuffer);

    char mqttLastWillTopic[64];
    snprintf(mqttLastWillTopic, sizeof(mqttLastWillTopic), "%s/%s", mqttBaseTopic, MQTT_LASTWILL_TOPIC);

    bbqConfigModified |= bbqConfig.putNotContains("setPoint", PV(20.f));

    controllerConfigModified |= controllerConfig.putNotContains("fan1Type", PV(0)); // 0=PWM 1=OnOff
    controllerConfigModified |= controllerConfig.putNotContains("fan1DutyCycle", PV(30 * 1000));
    controllerConfigModified |= controllerConfig.putNotContains("fan1Start", PV(50));

    controllerConfigModified |= controllerConfig.putNotContains("sensor1Type", PV(0)); // 0 == MAX31856 1 == MAX31855 2 = NTC
    controllerConfigModified |= controllerConfig.putNotContains("sensor2Type", PV(2));

    controllerConfigModified |= controllerConfig.putNotContains("NTC1Pin", PV(36));
    controllerConfigModified |= controllerConfig.putNotContains("NTC1Stein", PV("r1=10000, c1=-.0050990868, c2=0.0011737742, c3=-0.0000031896162, o=0.0"));

    controllerConfigModified |= controllerConfig.putNotContains("NTC2Pin", PV(36));
    controllerConfigModified |= controllerConfig.putNotContains("NTC2Stein", PV("r1=10000, c1=-.0050990868, c2=0.0011737742, c3=-0.0000031896162, o=0.0"));

    controllerConfigModified |= controllerConfig.putNotContains("mqttClientID", PV(mqttClientID));
    controllerConfigModified |= controllerConfig.putNotContains("mqttServer", PV(""));
    controllerConfigModified |= controllerConfig.putNotContains("mqttUsername", PV(""));
    controllerConfigModified |= controllerConfig.putNotContains("mqttPassword", PV(""));
    controllerConfigModified |= controllerConfig.putNotContains("mqttPort", PV(1883));
    controllerConfigModified |= controllerConfig.putNotContains("statusJson", PV(true));
    controllerConfig.put("mqttBaseTopic", PV(mqttBaseTopic));
    controllerConfig.put("mqttLastWillTopic", PV(mqttLastWillTopic));
}

void setup() {
    //********** CHANGE PIN FUNCTION  TO GPIO **********
    //https://www.esp8266.com/wiki/doku.php?id=esp8266_gpio_pin_allocations
    //GPIO 1 (TX) swap the pin to a GPIO.
    //pinMode(1, FUNCTION_3);
    //GPIO 3 (RX) swap the pin to a GPIO.
    //pinMode(3, FUNCTION_3);
    //**************************************************
    // Needed for ESP32, otherwhise crash
#if defined(ESP32)
    WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP
#endif
    // Enable serial port
    Serial.begin(115200);
    delay(050);
    // load configurations
    loadConfig(CONTROLLER_CONFIG_FILENAME, controllerConfig);
    loadConfig(BBQ_CONFIG_FILENAME, bbqConfig);
    setupDefaults();

    setupIOHardware();
    displayController->init();
    displayController->handle();
    setupMQTT();
    setupWifiManager();
    setupWIFIReconnectManager();
    Serial.println(F("End Setup"));
    effectPeriodStartMillis = millis();
}

#define NUMBER_OF_SLOTS 10
void loop() {
    const uint32_t currentMillis = millis();
    displayController->handle();

    if (currentMillis - effectPeriodStartMillis >= EFFECT_PERIOD_CALLBACK) {
        effectPeriodStartMillis += EFFECT_PERIOD_CALLBACK;
        counter50TimesSec++;

        // DigitalKnob (the button) must be handled at 50 times/sec to correct handle presses and double presses
        digitalKnob.handle();

#if defined(GEEKKCREIT_OLED)
        analogIn -> handle();
#endif
        // Handle fan
        ventilator1->handle(currentMillis);
        bbqController -> handle(currentMillis);

        // once a second publish status to mqtt (if there are changes)
        if (counter50TimesSec % 50 == 0) {
            publishStatusToMqtt();
        }

        if (counter50TimesSec % 500 == 0) {
            // serializeProperties<32>(Serial, controllerConfig);
        }

        // Maintenance stuff
        uint8_t slot50 = 0;

        if (counter50TimesSec % NUMBER_OF_SLOTS == slot50++) {
            bootSequence->handle();
        } else if (counter50TimesSec % NUMBER_OF_SLOTS == slot50++) {
            mqttClient.loop();
        } else if (counter50TimesSec % NUMBER_OF_SLOTS == slot50++) {
            if (controllerConfigModified) {
                controllerConfigModified = false;
                saveConfig(CONTROLLER_CONFIG_FILENAME, controllerConfig);
            }
        } else if (counter50TimesSec % NUMBER_OF_SLOTS == slot50++) {
            saveBBQConfigHandler.handle();
        } else if (counter50TimesSec % NUMBER_OF_SLOTS == slot50++) {
            temperatureSensor1->handle();
        } else if (counter50TimesSec % NUMBER_OF_SLOTS == slot50++) {
            temperatureSensor2->handle();
        } else if (counter50TimesSec % NUMBER_OF_SLOTS == slot50++) {
            wm.process();
        } else if (shouldRestart != 0 && (currentMillis - shouldRestart >= 5000)) {
            shouldRestart = 0;
            ESP.restart();
        }
    }
}
