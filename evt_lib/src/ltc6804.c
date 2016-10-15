#include "chip.h"
#include "mcp2515.h"
#include "util.h"

#define BUFFER_SIZE                         (256)

static uint16_t pec15Table[256];
static const uint16_t CRC15_POLY = 0x4599;

/*
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
*/
// should be input 16
// MSB -> LSB
int16_t ltc6804_calculate_pec(uint8_t *data_bits, uint8_t bit_len) {

    // LSB -> MSB
    uint8_t pec[15] = {0,0,0,0,1,0,0,0,0,0,0,0,0,0,0};

    uint16_t i;
    for(i = 0; i < bit_len; i++) {
        uint8_t in0 = data_bits[i]^pec[14];
        uint8_t in3 = in0^pec[2];
        uint8_t in4 = in0^pec[3];
        uint8_t in7 = in0^pec[6];
        uint8_t in8 = in0^pec[7];
        uint8_t in10 = in0^pec[9];
        uint8_t in14 = in0^pec[13];

        pec[14] = in14;
        pec[13] = pec[12];
        pec[12] = pec[11];
        pec[11] = pec[10];
        pec[10] = in10;
        pec[9] = pec[8];
        pec[8] = in8;
        pec[7] = in7;
        pec[6] = pec[5];
        pec[5] = pec[4];
        pec[4] = in4;
        pec[3] = in3;
        pec[2] = pec[1];
        pec[1] = pec[0];
        pec[0] = in0;
    }

    uint16_t outPEC = 0;
    for(i = 15; i > 0; i--) {
        outPEC += pec[i-1];
        outPEC = outPEC << 1;
    }

    return outPEC; 
}

void ltc6804_init(void) {
    init_pec_table();
}

