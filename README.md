[![Build Status](https://api.travis-ci.org/rvt/bbq-controller.svg?branch=master)](https://www.travis-ci.org/rvt/bbq-controller)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

# bbq-controller
Fuzzy logic based BBQ controller

### Currently under heavy development 
* Implement menu for a few settings like change temperature or manual fan control
* Test analog input potentiometer (just need ot create the HW)
* Test digital thermometer max31865
* Test digital thermometer max31855
* Test PWM fan control

A Fuzzy logic based BBQ controller based on Fuzzy Logic with the following features:

* OLed display of temperature readout, setting and fan speed
* Get and Set Fuzzy logic sets configuration without re-compilation
* Store and Load last settings and confuguration in EEPROM for offline controlling your BBQ (no WIFI needed)
* Change temperature setting with analog knob or over MQTT 
* Ventilator override over MQTT
* Parts of code tested with catch2
* tested with OpenHAB + InfluxDB + Grafana
* Unit tested with catch2

# Note

This is still work in progress and has not yet been field tested and development has been done using a simulator during unit tests so here is my TODO:
* Connect and test both temperature sensors
* Connect and test fan controller
* Change PWM frequency for more accurate fan control
* Test Lid open detection
* Test and implement low choarcoal detection
* Implement 'stall' delection and alerting [More about stall](https://amazingribs.com/more-technique-and-science/more-cooking-science/understanding-and-beating-barbecue-stall-bane-all)

# Tests
- tested on wemos ESP8266

# Run unit tests (requires cmake to be installed)
- ```cd libtest```
- ```mkdir build```
- ```cmake ../```
- ```make```
- ```./tests```

# Compilation
To build your version use platform io for compilation copy
```setup_example.h``` to ```setup.h``` and modify the contents to your needs.
Then run ```pio run``` to compile the binary. This will download the needed dependencies.

To upload to your wemos device run the following command (OSX):
```platformio run --target upload -e wemos --upload-port /dev/cu.SLAB_USBtoUART```

