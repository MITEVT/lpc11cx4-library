#ifndef _LTC6804_H_
#define _LTC6804_H_

#include "chip.h"
#include <string.h>
#include <stdint.h>

/* =================== TIMING MACROS ================== */

#define T_REFUP 5
#define T_SLEEP 1800
#define T_IDLE 4
#define T_WAKE 1

/* =================== COMMAND CODES ================== */

#define WRCFG 0x001
#define RDCFG 0x002
// [TODO] precalculate Command PECs

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

/***************************************
		Public Types
****************************************/

typedef enum {
	LTC6804_ADC_MODE_FAST = 1, LTC6804_ADC_MODE_NORMAL = 2, LTC6804_ADC_MODE_SLOW = 3
} LTC6804_ADC_MODE_T;

static const uint8_t LTC6804_ADC_MODE_WAIT_TIMES[] = {
	0, 2, 3, 202
};

static const uint16_t LTC6804_SELF_TEST_RES[] = {
	0, 0x9565, 0x9555, 0x9555
};

typedef struct {
	LPC_SSP_T *pSSP;
	uint32_t baud;
	uint8_t cs_gpio;
	uint8_t cs_pin;

	uint8_t num_modules;
	uint8_t *module_cell_count;

	uint32_t min_cell_mV;
	uint32_t max_cell_mV;

	LTC6804_ADC_MODE_T adc_mode;
} LTC6804_CONFIG_T;

typedef struct {
	Chip_SSP_DATA_SETUP_T *xf;
	uint8_t *tx_buf;
	uint8_t *rx_buf;
	uint32_t last_message;
	uint32_t wake_length;
	uint8_t *cfg;

	bool waiting;
	uint32_t wait_time;
	uint32_t flight_time;
	uint32_t last_sleep_wake;
} LTC6804_STATE_T;

typedef enum {
	LTC6804_WAITING, LTC6804_PASS, LTC6804_FAIL, LTC6804_SPI_ERROR, LTC6804_PEC_ERROR, LTC6804_WAITING_REFUP
} LTC6804_STATUS_T;

typedef struct {
	uint32_t *cell_voltages_mV; // array size = #modules * cells/module
	uint32_t pack_cell_max_mV;
	uint32_t pack_cell_min_mV;
} LTC6804_ADC_RES_T;


/***************************************
	Public Function Prototypes
****************************************/

LTC6804_STATUS_T LTC6804_Init(LTC6804_CONFIG_T *config, LTC6804_STATE_T *state, uint32_t msTicks);
LTC6804_STATUS_T LTC6804_WriteCFG(LTC6804_CONFIG_T *config, LTC6804_STATE_T *state, uint32_t msTicks);
bool LTC6804_VerifyCFG(LTC6804_CONFIG_T *config, LTC6804_STATE_T *state, uint32_t msTicks);
LTC6804_STATUS_T LTC6804_CVST(LTC6804_CONFIG_T *config, LTC6804_STATE_T *state, uint32_t msTicks);
LTC6804_STATUS_T LTC6804_SetBalanceStates(LTC6804_CONFIG_T *config, LTC6804_STATE_T *state, bool *balance_req, uint32_t msTicks);
LTC6804_STATUS_T LTC6804_GetCellVoltages(LTC6804_CONFIG_T *config, LTC6804_STATE_T *state, LTC6804_ADC_RES_T *res, uint32_t msTicks);
LTC6804_STATUS_T LTC6804_ClearCellVoltages(LTC6804_CONFIG_T *config, LTC6804_STATE_T *state, uint32_t msTicks);

#endif
