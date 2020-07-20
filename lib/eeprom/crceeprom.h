
#pragma once
#include <stdint.h>
#include <ESP_EEPROM.h>

extern "C" {
#include "crc16.h"
}

class CRCEEProm {
public:

    template <typename T> static int write(int p_eepromAddress, T& p_blob) {
        uint16_t crc = crc16(reinterpret_cast<uint8_t*>(&p_blob), sizeof(p_blob));
        EEPROM.put(p_eepromAddress, p_blob);
        EEPROM.put(p_eepromAddress + sizeof(p_blob), crc);
        return sizeof(p_blob) + sizeof(uint16_t);
    }

    template <typename T> static int size(T& p_blob) {
        return sizeof(p_blob) + sizeof(uint16_t);
    }

    template <typename T> static bool read(int p_eepromAddress, T& p_blob) {
        EEPROM.get(p_eepromAddress, p_blob);
        uint16_t crcRetreived;
        EEPROM.get(p_eepromAddress + sizeof(p_blob), crcRetreived);
        uint16_t crcCalculated = crc16(reinterpret_cast<uint8_t*>(&p_blob), sizeof(p_blob));
        return crcRetreived == crcCalculated;
    }

public:

};
