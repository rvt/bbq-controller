
#pragma once
#include <stdint.h>
#include <ESP_EEPROM.h>


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
    static uint16_t crc16(uint8_t* a, uint16_t length) {
        uint16_t crc = 0;

        for (uint16_t i = 0; i < length; i++) {
            crc = crc16Update(crc, a[i]);
        }

        return crc;
    }

    static uint16_t crc16Update(uint16_t crc, uint8_t a) {
        int i;
        crc ^= a;

        for (i = 0; i < 8; ++i)  {
            if (crc & 1) {
                crc = (crc >> 1) ^ 0xA001;
            } else {
                crc = (crc >> 1);
            }
        }

        return crc;
    }

};
