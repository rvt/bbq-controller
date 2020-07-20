#ifndef CRC16_H_INCLUDED
#define CRC16_H_INCLUDED

#include <stdint.h>


uint16_t crc16Update(uint16_t crc, uint8_t a) {
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

uint16_t crc16(uint8_t* a, uint16_t length) {
    uint16_t crc = 0;

    for (uint16_t i = 0; i < length; i++) {
        crc = crc16Update(crc, a[i]);
    }

    return crc;
}


#endif