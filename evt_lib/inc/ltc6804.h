#ifndef _LTC6804_H_
#define _LTC6804_H_

#include "lpc_types.h"

/* =================== COMMAND CODES ================== */

#define WRCFG 0x001
#define RDCFG 0x002
// [TODO] Precalculate Command PECs

#define RDCVA 0x004
#define RDCVB 0x006
#define RDCVC 0x008
#define RDCVD 0x00A
#define RDAUXA 0x00C
#define RDAUXB 0x00E
#define RDSTATA 0x010
#define RDSTATB 0x012

#define CLRCELL 0x711
#define CLRAUX 0x712
#define CLRSTAT 0x713
#define PLADC 0x714
#define DIAGN 0x715
#define WRCOMM 0x721
#define RDCOMM 0x722
#define STCOMM 0x723

/* =================== COMMAND BITS ================== */

#define ADC_MODE_7KHZ 0x10

#define ADC_CHN_ALL 0x0
#define ADC_CHN_1_7 0x1
#define ADC_CHN_2_8 0x2
#define ADC_CHN_3_9 0x3
#define ADC_CHN_4_10 0x4
#define ADC_CHN_5_11 0x5
#define ADC_CHN_6_12 0x6

#define PUP_DOWN 0x0
#define PUP_UP 0x1

#define ST_ONE 0x1
#define ST_TWO 0x2
#define ST_ONE_27KHZ_RES 0x9565
#define ST_ONE_14KHZ_RES 0x9553
#define ST_ONE_7KHZ_RES 0x9555
#define ST_ONE_2KHZ_RES 0x9555
#define ST_ONE_26HZ_RES 0x9555
#define ST_TWO_27KHZ_RES 0x6A9A
#define ST_TWO_14KHZ_RES 0x6AAC
#define ST_TWO_7KHZ_RES 0x6AAA
#define ST_TWO_2KHZ_RES 0x6AAA
#define ST_TWO_26HZ_RES 0x6AAA

#define NUM_CELL_GROUPS 4
/* =================== FUNCTION PROTOS ================== */

typedef struct CELL_INFO {
    uint16_t groupA[3];
    uint16_t groupB[3];
    uint16_t groupC[3];
    uint16_t groupD[3];
} CELL_INFO_T;

typedef enum CELL_GROUP {
    CELL_GROUP_A, CELL_GROUP_B, CELL_GROUP_C, CELL_GROUP_D
} CELL_GROUPS_T;


void LTC6804_Init(uint32_t baud, uint8_t cs_gpio, uint8_t cs_pin, uint32_t msTicks);
void LTC6804_ReadCFG(uint8_t *data, uint32_t msTicks);
void LTC6804_ReadVoltageGroup(uint8_t *rx_buf, CELL_INFO_T *readings, CELL_GROUPS_T cg, uint32_t msTicks);
void LTC6804_StartADC(uint32_t msTicks);

#endif
