#include "chip.h"
#include "mcp2515.h"
#include "util.h"

#define BUFFER_SIZE                         (256)

static uint16_t pec15Table[256];
static const uint16_t CRC15_POLY = 0x4599;


static void init_pec_table(void) {
    uint16_t remainder = 0;
    uint16_t i  = 0;
    for (i = 0; i < 256; i++) {
        remainder = i << 7;
        uint8_t bit = 0;
        for (bit = 8; bit > 0; --bit) {
            if (remainder & 0x4000) {
                remainder = ((remainder << 1));
                remainder = (remainder ^ CRC15_POLY);
            } else {
                remainder = ((remainder << 1));
            }
            pec15Table[i] = remainder & 0xFFFF;
        }
    }
}

// Based on LTC6804 Datasheet 41/Table 25
int16_t ltc6804_calculate_pec(char *data_bytes, uint8_t data_len) {

    uint16_t remainder = 16; // the PEC seed
    uint16_t address;
    
    uint16_t i;

    for (i = 0; i < data_len; i++) {
        // the PEC table address
        address = ((remainder >> 7) ^ data_bytes[i]) & 0xff;
        remainder = (remainder << 8 ) ^ pec15Table[address];
    }
    
    return remainder << 1; // zero is in LSB, so shifting to the left
}

void ltc6804_init(void) {
    init_pec_table();
}

