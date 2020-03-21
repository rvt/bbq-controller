/*
 */
#include <memory>
#include <cstring>
#include <vector>

#include "makestring.h"

#include <ESP8266WiFi.h>  // https://github.com/esp8266/Arduino
#include <ESP8266mDNS.h>
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include <FS.h>   // Include the SPIFFS library

#include <propertyutils.h>
#include <optparser.h>
#include <utils.h>
#include <icons.h>
#include <crceeprom.h>
#include <pwmventilator.h>
#include <stefanspwmventilator.h>
#include <onoffventilator.h>
#include <settings.h>

// #include <spi.h> // Include for harware SPI
#include <max31855sensor.h>
#include <max31865sensor.h>
#include <PubSubClient.h> // https://github.com/knolleary/pubsubclient/releases/tag/v2.6

#include "ssd1306displaycontroller.h"
#include <ESP_EEPROM.h>
#include <config.h>
#include <analogin.h>
#include <digitalknob.h>
#include <Fuzzy.h>

#include <bbqfanonly.h>
#include <bbq.h>
#include <demo.h>
#include <statemachine.h>

typedef PropertyValue PV ;

// of transitions
volatile uint32_t counter50TimesSec = 1;

// Number calls per second we will be handling
#define FRAMES_PER_SECOND        50
#define EFFECT_PERIOD_CALLBACK   (1000 / FRAMES_PER_SECOND)

// Keep track when the last time we ran the effect state changes
uint32_t effectPeriodStartMillis = 0;

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

// WiFI Manager
WiFiManager wm;

// Analog and digital inputs
std::shared_ptr<AnalogIn> analogIn = std::make_shared<AnalogIn>(0.2f);
DigitalKnob digitalKnob(BUTTON_PIN, true, 110);

// Stores information about the BBQ controller (PID values, fuzzy loggic values etc, mqtt)
Properties controllerConfig;
volatile bool controllerConfigModified = false;
// Stores information about the current temperature settings
Properties bbqConfig;
volatile bool bbqConfigModified = false;

// CRC value of last update to MQTT
volatile uint16_t lastMeasurementCRC = 0;
volatile uint32_t shouldRestart = 0;        // Indicate that a service requested an restart. Set to millies() of current time and it will restart 5000ms later

volatile bool hasMqttConfigured = false;
char* mqttLastWillTopic;
char* mqttClientID;
char* mqttSubscriberTopic;
uint8_t mqttSubscriberTopicStrLength;
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

bool saveConfigSPIFFS(const char* filename, Properties& properties);

Settings saveBBQConfigHandler(
    500,
    10000,
[]() {
    saveConfigSPIFFS(BBQ_CONFIG_FILENAME, bbqConfig);
    bbqConfigModified = false;
},
[]() {
    return bbqConfigModified;
}
);

///////////////////////////////////////////////////////////////////////////
//  Spiffs
///////////////////////////////////////////////////////////////////////////


bool loadConfigSpiffs(const char* filename, Properties& properties) {
    bool ret = false;

    if (SPIFFS.begin()) {
        Serial.println("mounted file system");

        if (SPIFFS.exists(filename)) {
            //file exists, reading and loading
            File configFile = SPIFFS.open(filename, "r");

            if (configFile) {
                Serial.print(F("Loading config : "));
                Serial.println(filename);
                deserializeProperties<32>(configFile, properties);
                //   serializeProperties<32>(Serial, properties);
            }

            configFile.close();
        } else {
            Serial.print(F("File not found: "));
            Serial.println(filename);
        }

        // SPIFFS.end();
    } else {
        Serial.print(F("Failed to begin SPIFFS"));
    }

    return ret;
}


/**
 * Store custom oarameter configuration in SPIFFS
 */
bool saveConfigSPIFFS(const char* filename, Properties& properties) {
    bool ret = false;

    if (SPIFFS.begin()) {
        SPIFFS.remove(filename);
        File configFile = SPIFFS.open(filename, "w");

        if (configFile) {
            Serial.print(F("Saving config : "));
            Serial.println(filename);
            serializeProperties<32>(configFile, properties);
            //                 serializeProperties<32>(Serial, properties);
            ret = true;
        } else {
            Serial.print(F("Failed to write file"));
            Serial.println(filename);
        }

        configFile.close();
        //    SPIFFS.end();
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
*/
void publishToMQTT(const char* topic, const char* payload);
void publishStatusToMqtt() {

    auto format = "to=%.2f t2=%.2f sp=%.2f f1=%.2f lo=%i lc=%i f1o=%.1f";
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
    uint16_t thisCrc = CRCEEProm::crc16((uint8_t*)buffer, std::strlen(buffer));

    if (thisCrc != lastMeasurementCRC) {
        publishToMQTT("status", buffer);
    }

    lastMeasurementCRC = thisCrc;
}

/**
 * Publish a message to mqtt
 */
void publishToMQTT(const char* topic, const char* payload) {
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
void handleCmd(const char* topic, const char* p_payload) {
    auto topicPos = topic + mqttSubscriberTopicStrLength;
    //Serial.print(F("Handle command : "));
    //Serial.println(topicPos);

    // Look for a temperature setPoint topic
    if (std::strstr(topicPos, "config") != nullptr) {
        BBQFanOnlyConfig config = bbqController->config();
        float temperature = 0;
        OptParser::get(p_payload, [&config, &temperature](OptValue values) {

            // Copy setpoint value
            if (std::strcmp(values.key(), "sp") == 0) {
                temperature = values;
            }

            // Fan On/Off controller duty cycle
            if (std::strcmp(values.key(), "ood") == 0) {
                int32_t v = values;
                v = between(v, (int32_t)5000, (int32_t)120000);
                controllerConfig.put("fOnOffDuty", PV(v));
                //ventilator1.reset(new OnOffVentilator(FAN1_PIN, (int32_t)controllerConfig.get("fOnOffDuty")));
            }

            // Lid open fan speed
            if (strcmp(values.key(), "lof") == 0) {
                int8_t v = values;
                config.fan_speed_lid_open = between(v, (int8_t) -1, (int8_t)100);
            }

            // Fan 1 override ( we don´t want this as an settings so if we loose MQTT connection we can always unplug)
            if (strcmp(values.key(), "f1o") == 0) {
                ventilator1->speedOverride(between((float)values, -1.0f, 100.0f));
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

        Serial.println("Config");
        // Update the bbqController with new values
        bbqController->config(config);
        bbqController->init();
    }

    if (strstr(topicPos, "reset") != nullptr) {
        OptParser::get(p_payload, [](OptValue v) {
            if (strcmp(v.key(), "1") == 0) {
                shouldRestart = millis();
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

    const char* mqttBaseTopic = controllerConfig.get("mqttBaseTopic");
    mqttClientID = makeCString("%08X", ESP.getChipId());
    mqttLastWillTopic = makeCString("%s/%s", mqttBaseTopic, MQTT_LASTWILL_TOPIC);
    mqttSubscriberTopic = makeCString("%s/+", mqttBaseTopic);
    mqttSubscriberTopicStrLength = std::strlen(mqttSubscriberTopic) - 1;
}

///////////////////////////////////////////////////////////////////////////
//  IOHardware
///////////////////////////////////////////////////////////////////////////

void setupIOHardware() {
    // Sensor 1 is generally used for the temperature of the bit
    auto sensor1 = new MAX31865sensor(SPI_MAX31865_CS_PIN, SPI_SDI_PIN, SPI_SDO_PIN, SPI_CLK_PIN, RNOMINAL_OVEN, RREF_OVEN);
    sensor1->begin(MAX31865_3WIRE);
    temperatureSensor1.reset(sensor1);

    // Sensor 2 is generally used to measure the temperature of the pit itself
    auto sensor2 = new Adafruit_MAX31855(SPI_CLK_PIN, SPI_MAX31855_CS_PIN, SPI_SDI_PIN);
    sensor2->begin();
    temperatureSensor2.reset(new MAX31855sensor(sensor2));

    ventilator1.reset(new StefansPWMVentilator(FAN1_PIN, (int16_t)controllerConfig.get("fStartPWM")));
    //    ventilator1.reset(new OnOffVentilator(FAN1_PIN, (int16_t)controllerConfig.get("fOnOffDuty")));
}

///////////////////////////////////////////////////////////////////////////
//  BBQT Controller
///////////////////////////////////////////////////////////////////////////

/**
 * Create the bbqController with required support hardware
 */
void setupBBQController() {
    bbqController.reset(new BBQFanOnly(temperatureSensor1, ventilator1));
    bbqController->init();
    bbqController->setPoint(bbqConfig.get("setPoint"));
}

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

        return 3;
    });
    CONNECTMQTT = new State([]() {
        mqttClient.setServer(
            controllerConfig.get("mqttServer"),
            (int16_t)controllerConfig.get("mqttPort")
        );

        if (mqttClient.connect(
                mqttClientID,
                controllerConfig.get("mqttUsername"),
                controllerConfig.get("mqttPassword"),
                mqttLastWillTopic,
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

    // set country
    wm.setClass("invert");
    wm.setCountry("US"); // setting wifi country seems to improve OSX soft ap connectivity, may help others as well
    wm.setWebServerCallback(serverOnlineCallback);
    wm.setConfigPortalTimeout(120);
    wm.addParameter(&wm_mqtt_server);
    wm.addParameter(&wm_mqtt_port);
    wm.addParameter(&wm_mqtt_user);
    wm.addParameter(&wm_mqtt_password);

    wm.setSaveParamsCallback(saveParamCallback);
    std::vector<const char*> menu = {"wifi", "wifinoscan", "info", "param", "sep", "erase", "restart"};
    wm.setMenu(menu);

    if (!wm.autoConnect("WM_AutoConnectAP")) {
        Serial.println("failed to connect and hit timeout");
        wm.startConfigPortal();
    }

    //if you get here you have connected to the WiFi
    wm.startWebPortal();
}

///////////////////////////////////////////////////////////////////////////
//  SETUP and LOOP
///////////////////////////////////////////////////////////////////////////

void setupDefaults() {
    bbqConfigModified |= bbqConfig.putNotContains("setPoint", PV(20.f));

    controllerConfigModified |= controllerConfig.putNotContains("fOnOffDuty", PV(30 * 1000));
    controllerConfigModified |= controllerConfig.putNotContains("fStartPWM", PV(50));
    controllerConfigModified |= controllerConfig.putNotContains("mqttBaseTopic", PV("BBQ"));
    controllerConfigModified |= controllerConfig.putNotContains("mqttServer", PV(""));
    controllerConfigModified |= controllerConfig.putNotContains("mqttUsername", PV(""));
    controllerConfigModified |= controllerConfig.putNotContains("mqttPassword", PV(""));
    controllerConfigModified |= controllerConfig.putNotContains("mqttPort", PV(1883));
}

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
    delay(250);
    // load configurations
    loadConfigSpiffs(CONTROLLER_CONFIG_FILENAME, controllerConfig);
    loadConfigSpiffs(BBQ_CONFIG_FILENAME, bbqConfig);
    setupDefaults();

    setupIOHardware();
    setupBBQController();
    ssd1306displayController.init();
    setupMQTT();
    setupWifiManager();
    setupWIFIReconnectManager();

    Serial.println(F("End Setup"));
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
                saveConfigSPIFFS(CONTROLLER_CONFIG_FILENAME, controllerConfig);
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
