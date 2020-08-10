// How often we are updating the mqtt state in ms
#ifndef MQTT_STATE_UPDATE_DELAY
#define MQTT_STATE_UPDATE_DELAY                       5000
#endif

#define MQTT_LASTWILL                           "lastwill"
#define MQTT_STATUS                           "status"
#define MQTT_LASTWILL_TOPIC                    "lastwill"
#define MQTT_LASTWILL_ONLINE                   "online"
#define MQTT_LASTWILL_OFFLINE                  "offline"

#if defined(GEEKKCREIT_OLED)
// Pin usages
// https://randomnerdtutorials.com/esp8266-pinout-reference-gpios/

// Device I used for my little projects
// https://www.arduino-tech.com/wemos-nodemcu-wifi-for-arduino-and-nodemcu-esp8266-0-96-inch-oled-board/

constexpr uint8_t WIRE_SDA = 5; // D1
constexpr uint8_t WIRE_SCL = 4; // D2

constexpr uint8_t FAN1_PIN = 15;  // D8 OUT PWM

// Pin 15 didn´t make the esp start up so we skipped it and took 2
constexpr uint8_t SPI_MAX31865_CS_PIN = 0;  // D3 OUT
constexpr uint8_t SPI_MAX31855_CS_PIN = 2; // D4 OUT

constexpr uint8_t BUTTON_PIN = 16; // D0 IN

/*
* Only tested is hardware SPI.
* Software SPI introduces long delays in Adafruit classes so try to avoid it at all cost
*/
constexpr int8_t SPI_SDO_PIN = -1; /* Set to -1 for HW SPI, connects to pin D6 12 on ESP8266 */
constexpr int8_t SPI_SDI_PIN = -1; /* Set to -1 for HW SPI, connects to pin D7 13 on ESP8266 */
constexpr int8_t SPI_CLK_PIN = -1; /* Set to -1 for HW SPI, connects to pin D5 14 on ESP8266 */

#define CONTROLLER_CONFIG_FILENAME "controllerCfg.conf"
#define BBQ_CONFIG_FILENAME "bbqCfg.conf"

#elif defined(TTG_T_DISPLAY)
// vhttps://github.com/espressif/arduino-esp32/issues/149
// esp32-hal.h add
// #define CONFIG_DISABLE_HAL_LOCKS 1

// Pin Layout https://github.com/Xinyuan-LilyGO/TTGO-T-Display
// https://nl.aliexpress.com/item/33048962331.html
constexpr uint8_t FAN1_PIN = 13;   // TOUCH4 OUT PWM
constexpr uint8_t BUTTON_PIN = 33; // TOICH5 IN
constexpr uint8_t ROTARY_PIN1 = 2; // TOICH5 IN
constexpr uint8_t ROTARY_PIN2 = 15; // TOICH5 IN

// Pin 15 didn´t make the esp start up so we skipped it and took 2
constexpr uint8_t SPI_MAX31865_CS_PIN = 37;  // OUT
constexpr uint8_t SPI_MAX31855_CS_PIN = 38;  // OUT

/*
* Software SPI from https://github.com/Xinyuan-LilyGO/TTGO-T-Display/issues/14
* More info on software SPI https://github.com/espressif/arduino-esp32/issues/149
*/
constexpr int8_t SPI_SDO_PIN = 27;
constexpr int8_t SPI_SDI_PIN = 26;
constexpr int8_t SPI_CLK_PIN = 25;

// Not in use for this environment
constexpr uint8_t WIRE_SDA = -1;
constexpr uint8_t WIRE_SCL = -1;

#define CONTROLLER_CONFIG_FILENAME "/controllerCfg.conf"
#define BBQ_CONFIG_FILENAME "/bbqCfg.conf"
#else
#error Unknown environment use TTG_T_DISPLAY or GEEKKCREIT_OLED
#endif

// The value of the Rref resistor. Use 430.0 for PT100 and 4300.0 for PT1000
constexpr float RREF_OVEN = 430.0;


// The 'nominal' 0-degrees-C resistance of the sensor
// 100.0 for PT100, 1000.0 for PT1000
constexpr float RNOMINAL_OVEN = 100.0;

// Period of ON/OF fan in milliseconds
constexpr uint32_t ON_OFF_FAN_PERIOD = (30 * 1000);

