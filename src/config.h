// How often we are updating the mqtt state in ms
#ifndef MQTT_STATE_UPDATE_DELAY
#define MQTT_STATE_UPDATE_DELAY                       5000
#endif

#define MQTT_LASTWILL                           "lastwill"
#define MQTT_STATUS                           "status"
#define MQTT_LASTWILL_TOPIC                    "lastwill"
#define MQTT_LASTWILL_ONLINE                   "online"
#define MQTT_LASTWILL_OFFLINE                  "offline"

// Base hostname, used for the MQTT Client ID and OTA hostname
#ifndef HOSTNAME_TEMPLATE
#define HOSTNAME_TEMPLATE                       "BBQ%s"
#endif

// Enable console output via telnet OR SERIAL
// #define ARILUX_DEBUG_TELNET
//#define DEBUG_SERIAL

// When set we will pause for any OTA messages before we startup, no commands are handled in this time
// #define PAUSE_FOR_OTA

constexpr uint8_t WIRE_SDA = 5;
constexpr uint8_t WIRE_SCL = 4;

constexpr uint8_t FAN1_PIN = 3;


/*
 * Only tested is hardware SPI.
 * Software SPI introduces long delays in Adafruit classes so try to avoid it at all cost
*/
constexpr int8_t SPI_SDO_PIN = -1; /* Set to -1 for HW SPI, connects to pin 12 on ESP8266 */

constexpr int8_t SPI_SDI_PIN = -1; /* Set to -1 for HW SPI, connects to pin 13 on ESP8266 */

constexpr int8_t SPI_CLK_PIN = -1; /* Set to -1 for HW SPI, connects to pin 14 on ESP8266 */

// Pin 15 didn´t make the esp start up so we skipped it and took 2

constexpr uint8_t SPI_MAX31865_CS_PIN = 2;
constexpr uint8_t SPI_MAX31855_CS_PIN = 15;

constexpr uint8_t BUTTON_PIN = 0;

// The value of the Rref resistor. Use 430.0 for PT100 and 4300.0 for PT1000
constexpr float RREF_OVEN = 430.0;


// The 'nominal' 0-degrees-C resistance of the sensor
// 100.0 for PT100, 1000.0 for PT1000
constexpr float RNOMINAL_OVEN = 100.0;

// Period of ON/OF fan in milliseconds
constexpr uint32_t ON_OFF_FAN_PERIOD = (30 * 1000);

#define CONTROLLER_CONFIG_FILENAME "controllerCfg.conf"
#define BBQ_CONFIG_FILENAME "bbqCfg.conf"