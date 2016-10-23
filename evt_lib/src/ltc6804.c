#include "chip.h"
#include "mcp2515.h"
#include "util.h"

#define BUFFER_SIZE                         (256)

static uint16_t pec15Table[256];

uint16_t ltc6804_calculate_pec(uint8_t *data, uint8_t len) {

	int i,j;
	uint16_t pec = 0x0010;

	for (i = 0; i < len; i++) {
		for (j = 0; j < 8; j++) {
			uint16_t din = data[i] >> (7-j) & 1;
			uint16_t in0 = din ^ (pec >> 14 & 1); 				//in0 = din ^ pec[14]
			uint16_t in3 = ((in0 << 2) ^ (pec & 0x0004)) << 1;	//in3 = in0 ^ pec[2]
			uint16_t in4 = ((in0 << 3) ^ (pec & 0x0008)) << 1;	//in4 = in0 ^ pec[3]
			uint16_t in7 = ((in0 << 6) ^ (pec & 0x0040)) << 1;	//in7 = in0 & pec[6]
			uint16_t in8 = ((in0 << 7) ^ (pec & 0x0080)) << 1;	//in8 = in0 & pec[7];
			uint16_t in10 = ((in0 << 9) ^ (pec & 0x0200)) << 1;	//in10 = in0 ^ pec[9]
			uint16_t in14 = ((in0 << 13) ^ (pec & 0x2000)) << 1; //in14 = in0 ^ pec[13];
			
			pec = (pec & ~0x4000) + in14;					//pec14 = in14
			pec = (pec & ~0x2000) + ((pec & 0x1000) << 1);	//pec13 = pec12
			pec = (pec & ~0x1000) + ((pec & 0x0800) << 1);	//pec12 = pec11
			pec = (pec & ~0x0800) + ((pec & 0x0400) << 1);	//pec11 = pec10
			pec = (pec & ~0x0400) + in10;					//pec10 = in10
			pec = (pec & ~0x0200) + ((pec & 0x0100) << 1);	//pec9 = pec8
			pec = (pec & ~0x0100) + in8;					//pec8 = in8
			pec = (pec & ~0x0080) + in7;					//pec7 = in7
			pec = (pec & ~0x0040) + ((pec & 0x0020) << 1);	//pec6 = pec5
			pec = (pec & ~0x0020) + ((pec & 0x0010) << 1);	//pec5 = pec4
			pec = (pec & ~0x0010) + in4;					//pec4 = in4;
			pec = (pec & ~0x0008) + in3; 					//pec3 = in3
			pec = (pec & ~0x0004) + ((pec & 0x0002) << 1); 	//pec2 = pec1
			pec = (pec & ~0x0002) + ((pec & 0x0001) << 1); 	//pec1 = pec0
			pec = (pec & ~0x0001) + in0; 					//pec0 = in0
		}
		
	}

	return pec << 1;

}

