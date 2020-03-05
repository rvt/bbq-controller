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

#ifndef WIRE_SDA
#define WIRE_SDA 5
#endif

#ifndef WIRE_SCL
#define WIRE_SCL 4
#endif


#ifndef FAN1_PIN
#define FAN1_PIN 3
#endif


/*
 * Only tested is hardware SPI.
 * Software SPI introduces long delays in Adafruit classes so try to avoid it at all cost
*/
#ifndef SPI_SDO_PIN
#define SPI_SDO_PIN -1 /* Set to -1 for HW SPI, connects to pin 12 on ESP8266 */
#endif

#ifndef SPI_SDI_PIN
#define SPI_SDI_PIN -1 /* Set to -1 for HW SPI, connects to pin 13 on ESP8266 */
#endif

#ifndef SPI_CLK_PIN
#define SPI_CLK_PIN -1 /* Set to -1 for HW SPI, connects to pin 14 on ESP8266 */
#endif

// Pin 15 didn´t make the esp start up so we skipped it and took 2
#ifndef SPI_MAX31865_CS_PIN
#define SPI_MAX31865_CS_PIN 2
#endif

#ifndef SPI_MAX31855_CS_PIN
#define SPI_MAX31855_CS_PIN 15
#endif

#ifndef BUTTON_PIN
#define BUTTON_PIN 0
#endif

// The value of the Rref resistor. Use 430.0 for PT100 and 4300.0 for PT1000
#ifndef RREF_OVEN
#define RREF_OVEN      430.0
#endif

// The 'nominal' 0-degrees-C resistance of the sensor
// 100.0 for PT100, 1000.0 for PT1000
#ifndef RNOMINAL_OVEN
#define RNOMINAL_OVEN  100.0
#endif

#ifndef PWM_FAN
#define PWM_FAN 1
#endif

#ifndef ON_OFF_FAN
#define ON_OFF_FAN 1
// Period of ON/OF fan in milliseconds
#define ON_OFF_FAN_PERIOD (30 * 1000)
#endif

#define CONTROLLER_CONFIG_FILENAME "controllerCfg.json"
#define BBQ_CONFIG_FILENAME "bbqCfg.json"