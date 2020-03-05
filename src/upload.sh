
#List mDNS devices
#platformio device list --mdns --logical | grep arduino


# ping ARILUX00AB5E97.local
platformio device monitor

# ping ARILUX00879231.local

#platformio device monitor --baud 115200
#platformio run --target upload -e wemos --upload-port /dev/cu.SLAB_USBtoUART

# connect serial IO
#platformio device monitor

#platformio test -e native
