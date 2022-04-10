#!/bin/bash

set -e 

#List mDNS devices
#platformio device list --mdns --logical | grep arduino


# ping ARILUX00AB5E97.local

# ping ARILUX00879231.local
#./generateHtmlArray.sh 
#platformio run
pio run --target upload -e ttgo-t-display
pio device monitor --baud 115200

echo "
#include <WebServer.h>
#include <DNSServer.h>
"
# connect serial IO
#platformio device monitor

#platformio test -e native
