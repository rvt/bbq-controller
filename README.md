# bbq-controller
Fuzzy logic based BBQ controller

A Fuzzy logec based BBQ controller based on Fuzzy Logic with the following features:

* Control Fuzzy Logic parameters while operating a BBQ
* USe MQQT to control all parameters and setpoint
* Second update over MQTT
* Store and Load last settings in EEPROM for offline controlling your BBQ
* Change temperature setting with analog knob
* Parts tested with catch2

# Tested
- tested on wemos ESP8266

# Run unit tests
- ```cd libtest```
- ```mkdir build```
- ```cmake ../```
- ```make```
- ```./tests```

# Compilation
To build your version use platform io for compilation copy
```setup_example.h``` to ```setup.h``` and modify the contents to your needs.
Then run ```pio run``` to compile the binary. This will download the needed dependencies.

To upload to your ESP run the following command (OSX):
```platformio run --target upload -e wemos --upload-port /dev/cu.SLAB_USBtoUART```

