/* crc.c */


#include <stdint.h>
#include "crc.h"



/* CRC-CCITT (Kermit) */
uint16_t crc16(uint16_t crc, volatile uint8_t *ptr, unsigned int length) {
    int i;

    while (length--) {
        crc ^= *ptr++;
        for (i = 0; i < 8; i++) {
            if (crc & 0x0001) {
                crc >>= 1;
                crc ^= 0x8408;
            } else
                crc >>= 1;
        }
    }

    return crc;
}

