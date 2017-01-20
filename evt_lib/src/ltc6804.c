#include "chip.h"
#include "ltc6804.h"
#include <string.h>

#define SPI_BUFFER_SIZE 12

static uint8_t CFG[6];
static uint32_t _last_message = 2000;
static uint8_t _wake_length = 0;

static uint8_t Tx_Buf[SPI_BUFFER_SIZE];
static Chip_SSP_DATA_SETUP_T xf_setup;
static uint8_t _cs_gpio;
static uint8_t _cs_pin;
static uint32_t _baud;
static LPC_SSP_T *_pSSP;

uint16_t LTC6804_CalculatePEC(uint8_t *data, uint8_t len);

void LTC6804_Wake(uint32_t msTicks);

void LTC6804_LoadCFG(uint32_t msTicks);

void LTC6804_Wake(uint32_t msTicks) {
	if (msTicks - _last_message < 1500) return;

	uint8_t i;
	for (i = 0; i < _wake_length; i++) {
		Tx_Buf[i] = 0x00;
	}

	_last_message = msTicks;
	Chip_GPIO_SetPinState(LPC_GPIO, _cs_gpio, _cs_pin, false);
	Chip_SSP_WriteFrames_Blocking(_pSSP, Tx_Buf, _wake_length);
	Chip_GPIO_SetPinState(LPC_GPIO, _cs_gpio, _cs_pin, true);
}

void LTC6804_LoadCFG(uint32_t msTicks) {
	Tx_Buf[0] = WRCFG >> 8;
	Tx_Buf[1] = WRCFG & 0xFF;
	uint16_t pec = LTC6804_CalculatePEC(Tx_Buf, 2);
	Tx_Buf[2] = pec >> 8;
	Tx_Buf[3] = pec & 0xFF;
	memcpy(Tx_Buf+4, CFG, 6);
	pec = LTC6804_CalculatePEC(Tx_Buf+4, 6);
	Tx_Buf[10] = pec >> 8;
	Tx_Buf[11] = pec & 0xFF;

	LTC6804_Wake(msTicks);

	_last_message = msTicks;
	Chip_GPIO_SetPinState(LPC_GPIO, _cs_gpio, _cs_pin, false);
	Chip_SSP_WriteFrames_Blocking(_pSSP, Tx_Buf, 12);
	Chip_GPIO_SetPinState(LPC_GPIO, _cs_gpio, _cs_pin, true);
}

void LTC6804_Init(LPC_SSP_T *pSSP, uint32_t baud, uint8_t cs_gpio, uint8_t cs_pin, uint32_t msTicks) {
	_pSSP = pSSP;
	_baud = baud;
	_cs_gpio = cs_gpio;
	_cs_pin = cs_pin;
	_last_message = msTicks;
	_wake_length = _baud/80000;
	_last_message = 2000;

	Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO2_2, (IOCON_FUNC2 | IOCON_MODE_INACT));	/* MISO1 */ 
    Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO2_3, (IOCON_FUNC2 | IOCON_MODE_INACT));	/* MOSI1 */
	Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO2_1, (IOCON_FUNC2 | IOCON_MODE_INACT));	/* SCK1 */
	Chip_GPIO_WriteDirBit(LPC_GPIO, _cs_gpio, _cs_pin, true);							/* Chip Select */
	Chip_GPIO_SetPinState(LPC_GPIO, _cs_gpio, _cs_pin, true);

	Chip_SSP_Init(_pSSP);
	Chip_SSP_SetBitRate(_pSSP, _baud);
	Chip_SSP_SetFormat(_pSSP, SSP_BITS_8, SSP_FRAMEFORMAT_SPI, SSP_CLOCK_MODE3);
	Chip_SSP_SetMaster(_pSSP, true);
	Chip_SSP_Enable(_pSSP);

	xf_setup.tx_data = Tx_Buf;

	// Default Configuration of GPIO Pulldown off, ADCOPT = 0, VUV = 0, VOV = 0, REFON = 1, Discharge Off w/ no timeout
	// [TODO] Set default by config.h instead
	CFG[0] = 0xFC;
	CFG[1] = 0x00;
	CFG[2] = 0x00;
	CFG[3] = 0x00;
	CFG[4] = 0x00;
	CFG[5] = 0x00;

	LTC6804_LoadCFG(msTicks);
}

void LTC6804_StartADC(uint32_t msTicks) {
    // Assumes reading from all cell voltages CH[0:2] = 0
    //  and DCP = 0, and 26Hz filtered = MD = 0b11
	Tx_Buf[0] = 0x03;
	Tx_Buf[1] = 0x60;

	uint16_t pec = LTC6804_CalculatePEC(Tx_Buf, 2);
	Tx_Buf[2] = pec >> 8;
	Tx_Buf[3] = pec & 0xFF;

	LTC6804_Wake(msTicks);

	_last_message = msTicks;
	Chip_GPIO_SetPinState(LPC_GPIO, _cs_gpio, _cs_pin, false);
	Chip_SSP_WriteFrames_Blocking(_pSSP, Tx_Buf, 4);
	Chip_GPIO_SetPinState(LPC_GPIO, _cs_gpio, _cs_pin, true);
}

void LTC6804_CVSelfTestCmd(uint32_t msTicks) {
    Tx_Buf[0] = 0x03;
    Tx_Buf[1] = 0x27;
	uint16_t pec = LTC6804_CalculatePEC(Tx_Buf, 2);
    Tx_Buf[2] = pec >> 8;
    Tx_Buf[3] = pec & 0xFF;

	LTC6804_Wake(msTicks);

	_last_message = msTicks;
	Chip_GPIO_SetPinState(LPC_GPIO, _cs_gpio, _cs_pin, false);
	Chip_SSP_WriteFrames_Blocking(_pSSP, Tx_Buf, 4);
	Chip_GPIO_SetPinState(LPC_GPIO, _cs_gpio, _cs_pin, true);
}

void LTC6804_OpenWireTestCmd(uint8_t pup_bit, uint32_t msTicks) {
	Tx_Buf[0] = 0x03;
    if(pup_bit == 0) {
	    Tx_Buf[1] = 0x28;
    } else {
	    Tx_Buf[1] = 0x68;
    }
	uint16_t pec = LTC6804_CalculatePEC(Tx_Buf, 2);
	Tx_Buf[2] = pec >> 8;
	Tx_Buf[3] = pec & 0xFF;

	LTC6804_Wake(msTicks);

	_last_message = msTicks;
	Chip_GPIO_SetPinState(LPC_GPIO, _cs_gpio, _cs_pin, false);
	Chip_SSP_WriteFrames_Blocking(_pSSP, Tx_Buf, 4);
	Chip_GPIO_SetPinState(LPC_GPIO, _cs_gpio, _cs_pin, true);
}

static void ZeroTxBuf(uint8_t start, uint8_t end) {
	uint8_t i;
	for (i = start; i < end; i++) {
		Tx_Buf[i] = 0;
	}

}

void LTC6804_ReadCFG(uint8_t *data, uint32_t msTicks) {
	Tx_Buf[0] = RDCFG >> 8;
	Tx_Buf[1] = RDCFG & 0xFF;
	uint16_t pec = LTC6804_CalculatePEC(Tx_Buf, 2);
	Tx_Buf[2] = pec >> 8;
	Tx_Buf[3] = pec & 0xFF;

    ZeroTxBuf(4, 12);

	xf_setup.length = 12; xf_setup.rx_cnt = 0; xf_setup.tx_cnt = 0;
	xf_setup.rx_data = data;

	LTC6804_Wake(msTicks);

	_last_message = msTicks;
	Chip_GPIO_SetPinState(LPC_GPIO, _cs_gpio, _cs_pin, false);
	Chip_SSP_RWFrames_Blocking(_pSSP, &xf_setup);
	Chip_GPIO_SetPinState(LPC_GPIO, _cs_gpio, _cs_pin, true);
}

void LTC6804_ReadVoltageGroup(uint8_t *rx_buf, CELL_INFO_T *readings, CELL_GROUPS_T cg, uint32_t msTicks) {

	Tx_Buf[0] = 0x00;
    if(cg == CELL_GROUP_A) {
	    Tx_Buf[1] = RDCVA;
    } else if(cg == CELL_GROUP_B) {
	    Tx_Buf[1] = RDCVB;
    } else if(cg == CELL_GROUP_C) {
	    Tx_Buf[1] = RDCVC;
    } else {
	    Tx_Buf[1] = RDCVD;
    }

	uint16_t pec = LTC6804_CalculatePEC(Tx_Buf, 2);
	Tx_Buf[2] = pec >> 8;
	Tx_Buf[3] = pec & 0xFF;

    ZeroTxBuf(4, 12);

	xf_setup.length = 12;
	xf_setup.tx_cnt = 0;
	xf_setup.rx_data = rx_buf;

	LTC6804_Wake(msTicks);

	_last_message = msTicks;
	Chip_GPIO_SetPinState(LPC_GPIO, _cs_gpio, _cs_pin, false);
	Chip_SSP_RWFrames_Blocking(_pSSP, &xf_setup);
	Chip_GPIO_SetPinState(LPC_GPIO, _cs_gpio, _cs_pin, true);

    if(cg == CELL_GROUP_A) {
	    readings->groupA[0] = (rx_buf[4])|(rx_buf[5]<<8);
	    readings->groupA[1] = (rx_buf[6])|(rx_buf[7]<<8);
	    readings->groupA[2] = (rx_buf[8])|(rx_buf[9]<<8);
    } else if(cg == CELL_GROUP_B) {
	    readings->groupB[0] = (rx_buf[4])|(rx_buf[5]<<8);
	    readings->groupB[1] = (rx_buf[6])|(rx_buf[7]<<8);
	    readings->groupB[2] = (rx_buf[8])|(rx_buf[9]<<8);
    } else if(cg == CELL_GROUP_C) {
	    readings->groupC[0] = (rx_buf[4])|(rx_buf[5]<<8);
	    readings->groupC[1] = (rx_buf[6])|(rx_buf[7]<<8);
	    readings->groupC[2] = (rx_buf[8])|(rx_buf[9]<<8);
    } else {
	    readings->groupD[0] = (rx_buf[4])|(rx_buf[5]<<8);
	    readings->groupD[1] = (rx_buf[6])|(rx_buf[7]<<8);
	    readings->groupD[2] = (rx_buf[8])|(rx_buf[9]<<8);
    }
}

uint16_t LTC6804_CalculatePEC(uint8_t *data, uint8_t len) {

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
