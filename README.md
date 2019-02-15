[![Build Status](https://api.travis-ci.org/rvt/bbq-controller.svg?branch=master)](https://www.travis-ci.org/rvt/bbq-controller)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

# bbq-controller
Fuzzy logic based BBQ controller

### Currently under heavy development 
* Test digital thermometer max31855
* Test PWM fan control
* Implement Farenheit display

A Fuzzy logic based BBQ controller based on Fuzzy Logic with the following features:

* Oled display of temperature readout, setting and fan speed
* Get and Set Fuzzy logic sets configuration without re-compilation
* Store and Load last settings and confuguration in EEPROM for offline controlling your BBQ (no WIFI needed)
* Change temperature setting with build in menu or over MQTT 
* Ventilator override over MQTT or via menu
* Parts of code tested with catch2
* Tested with OpenHAB + InfluxDB + Grafana


# Note

This is still work in progress and has not yet been field tested and development has been done using a simulator during unit tests so here is my TODO:
* Connect and test MAX31855 temperature sensor for meat monitor
* Connect and test fan controller
* Test Lid open detection
* Test and implement low choarcoal detection
* Implement 'stall' delection and alerting [More about stall](https://amazingribs.com/more-technique-and-science/more-cooking-science/understanding-and-beating-barbecue-stall-bane-all)

# Tests
- Tested on wemos ESP8266 [Wemos® Nodemcu Wifi And ESP8266 NodeMCU + 1.3 Inch OLED](https://www.banggood.com/Wemos-Nodemcu-Wifi-And-ESP8266-NodeMCU-1_3-Inch-OLED-Board-White-p-1160048.html)

# Run unit tests (requires cmake to be installed)
- ```cd libtest```
- ```mkdir build```
- ```cmake ../```
- ```make```
- ```./tests```

See also [https://travis-ci.org/rvt/bbq-controller](https://travis-ci.org/rvt/bbq-controller)

# Compilation
To build your version use platform io for compilation: Copy
```setup_example.h``` to ```setup.h``` and modify the contents to your needs so it can connect to your wifi and potentioally your MQTT broker.
Then run ```pio run``` to compile the binary. This will download the needed dependencies.

To upload to your wemos device run the following command (OSX):
```platformio run --target upload -e wemos --upload-port /dev/cu.SLAB_USBtoUART```

# MQTT Messages

MQTT is used to change it´s configuration and monitor your BBQ (along side the OLED display if used).

I use [MQTT Spy](https://github.com/eclipse/paho.mqtt-spy/releases)  to spy for messages and use OpenHAB to push the status messages into influxdb. Since the controller only talks to a MQTT message broker, you are free to use any other tools for that, I hear that [Node-RED](https://nodered.org) is pretty cool.

### Status messages

Topic: ```BBQ/XXXXXXXX/config/state```

Status messages are send to an MQTT broker each time any of the following variables changed. Status messages are great to monitor the progress during cooking.

| name | type  |  Meaning | value  | Unit  | Note |
|---   |---    |---       |---     |---    |---   |
| to   | float | Temperature sensor 1, usually pit temp  | 0..250 | celsius | |
| t2   | float | Temperature sensor 2, usually pit temp  | 0..250 | celsius | |
| sp   | float | Setpoint temperature  | 90..240  | celsius  | |
| f1   | float | Ventilator 1 speed  | 0..100  | % | |
| f1o  | float | Ventilator 1 speed override  | -1..100  | %  | When set > -1 it´s in override mode |
| lo   | bool | Lid Open detection  | 0 or 1  | 1 When lid open is detected  |
| lc   | bool | Low Charcoal detection  | 0 or 1  | 1 When low charcoal is detected |

Example message:
```to=130.5 t2=60.2 sp=130.0 f1=25 lo=0 lc=0 f1o=-1```

### Config messages

Topic: ```BBQ/XXXXXXXX/config```
Topic: ```BBQ/XXXXXXXX/config/state``` each time any of the variables are changed the configuration is published to this topic

The controller uses a single topic to configure the behavior.

| name | type  |  Meaning | value  | Default | Unit  | Note |
|---   |---    |---       |---     |---      |---    |---   |
| sp   | float | Set the desired temperature | 90..240 | 30.0 | Celsius |
| fs1   | int | set Minimum fan speed in % | 0..100 | 20 | % | Some fan's don't start with low PWM values, here set the minimum % of value where the fan wil start|
| f1o  | float | Override fan 1 | -1-100 | -1 | % | -1 will set it to auto mode, eg let the controller handle the speed. Any value > -0.5 will be in override |
| fl1  | float,float,float,float | Fuzzy set for Low Fan |  0-100 | 0.0,0.0,0.0,50.0 | % | |
| fm1  | float,float,float,float | Fuzzy set for Medium Fan |  0-100 | 25.0,50.0,50.0,75.0 | % | |
| fh1  | float,float,float,float | Fuzzy set for Hight Fan |  0-100 | 50.0,100.0,100.0,100.0 | % | |
| tel  | float,float,float,float | Fuzzy set for low temperature error |  0-XX | 0.0,10.0 | Celsius | |
| tem  | float,float,float,float | Fuzzy set for medium temperature error |  0-XX | 0.0,15.0,15.0,30.0 | Celsius | |
| teh  | float,float,float,float | Fuzzy set for high temperature error |  0-XX | 15.0,200.0,200.0,200.0 | Celsius | |
| tcf  | float,float,float,float | Fuzzy set for temperature drop detection |  0-XX | 10.0,20.0,20.0,30.0 | Celsius | |

Example messages to ```BBQ/XXXXXXXX/config```:

* ```sp=130.0``` Set the desired pit temp to 130 degree Celsius
* ```f1o=55``` Override fan speed to 55%
* ```f1o=-1 sp=180.0``` Enable auto mode and set desired temp to 180Celsius in one configuration line
* ```fl1=0.0,0.0,0.0,50.0``` Update Fuzzy Set for fan 1, this will re-configure the BBQ controller

### fs1 with PWM
Ventilators controlled by PWM do have an issue that they don´t run very well on lower ranges, or they won´t start up well.
The PWMVentilator class will alow a minimal usable value where the ventilator is usable.
Issuing fs1=30 will allow to run to a minimum of 30% PWM range.

That means that when when the controller issues 1% fan speed, it get´ translated to 30% PWM value,
so the actual range will be translated from 0%..100% (what you see on display) to 30%..100% PWM range.
In addition when you start from 0% to it will issue a 100ms delay at 100% to allow the fan to start up.
note: The 100ms delay is temporary hack

## Hardware needed (under construction)

* Wemos® Nodemcu Wifi And ESP8266 NodeMCU + 1.3 Inch OLED
* Linear 10K Potentiometer
* Push Button
* Theromcouple for meet temperature measurement
* Thermocouple for Pit temperature measurement [RTD Pt100](https://www.banggood.com/RTD-Pt100-Temperature-Sensor-2m-Cable-Probe-98mm-3-Wires-50400Degree-p-923736.html?rmmds=search)
* Sensor module for meat Probe [MAX31855](https://www.banggood.com/MAX31855-MAX6675-SPI-K-Thermocouple-Temperature-Sensor-Module-Board-For-Arduino-p-1193988.html?rmmds=search&cur_warehouse=CN)
* Sensor module for pit (PT100) probe [MAX31865](https://www.banggood.com/GY-31865-MAX31865-Temperature-Sensor-Module-RTD-Digital-Conversion-Module-p-1416434.html?rmmds=search&cur_warehouse=CN)
* 5V Ventilator 
* Some box to put it all in

If you use the above hardware you have to re-configure the MAX31865 sensor module for 3-Wrire configuration. This is described on this page : [Adafruit 4-Wire RTDs](https://learn.adafruit.com/adafruit-max31865-rtd-pt100-amplifier/rtd-wiring-config)

Additional documentation from the official website:

[MAX31865](https://www.maximintegrated.com/en/products/sensors/MAX31865.html)
[MAX31855](https://www.maximintegrated.com/en/products/sensors/MAX31855.html)

# Credits

* AJ Alves - Fuzzy Logic [eFLL (Embedded Fuzzy Logic Library)](https://github.com/zerokol/eFLL)
* [Adafruit Libraries](https://www.adafruit.com/)
* Knolleary PubSub Client [Pub Sub client](https://pubsubclient.knolleary.net)
* Faster i2c [Brzo i2c](https://github.com/pasko-zh/brzo_i2c/wiki)
* ESP8266 OLED [SSD1306](https://github.com/squix78/esp8266-oled-ssd1306)
* ESP EEPROM helper library [ESP EEPROM](https://github.com/jwrw/ESP_EEPROM)
* PT1000 Library [pt100 rtd](https://github.com/drhaney/pt100rtd)