#include "chip.h"
#include "config.h"
#include "lc1024.h"
#include <string.h>

static uint8_t Tx_Buf[SPI_BUFFER_SIZE];
static Chip_SSP_DATA_SETUP_T xf_setup;
uint8_t _cs_gpio;
uint8_t _cs_pin;
uint32_t _baud;

void ZeroTxBuf(uint8_t start);

void LC1024_Init(uint32_t baud, uint8_t cs_gpio, uint8_t cs_pin) {

	_baud = baud;
	_cs_gpio = cs_gpio;
	_cs_pin = cs_pin;

	Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO2_2, (IOCON_FUNC2 | IOCON_MODE_INACT));	/* MISO1 */ 
    Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO2_3, (IOCON_FUNC2 | IOCON_MODE_INACT));	/* MOSI1 */
	Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO2_1, (IOCON_FUNC2 | IOCON_MODE_INACT));	/* SCK1 */
	Chip_GPIO_WriteDirBit(LPC_GPIO, _cs_gpio, _cs_pin, true);							/* Chip Select */
	Chip_GPIO_SetPinState(LPC_GPIO, _cs_gpio, _cs_pin, true);

	Chip_SSP_Init(LPC_SSP1);
	Chip_SSP_SetBitRate(LPC_SSP1, _baud);
	Chip_SSP_SetFormat(LPC_SSP1, SSP_BITS_8, SSP_FRAMEFORMAT_SPI, SSP_CLOCK_MODE3);
	Chip_SSP_SetMaster(LPC_SSP1, true);
	Chip_SSP_Enable(LPC_SSP1);

	xf_setup.tx_data = Tx_Buf;
    ZeroTxBuf(0);
}

void ZeroTxBuf(uint8_t start) {
	uint8_t i;
	for (i = start; i < SPI_BUFFER_SIZE; i++) {
		Tx_Buf[i] = 0;
	}
}

void LC1024_ReadStatusReg(uint8_t* data) {
    Tx_Buf[0] = RD_STATUS_REG_INSTR;
    ZeroTxBuf(1);
	xf_setup.length = 2; xf_setup.rx_cnt = 0; xf_setup.tx_cnt = 0;
	xf_setup.rx_data = data;

	Chip_GPIO_SetPinState(LPC_GPIO, _cs_gpio, _cs_pin, false);
	Chip_SSP_RWFrames_Blocking(LPC_SSP1, &xf_setup);
	Chip_GPIO_SetPinState(LPC_GPIO, _cs_gpio, _cs_pin, true);
}

void LC1024_WriteDisable(void) {
    Tx_Buf[0] = WRITE_DISABLE_INSTR;
    ZeroTxBuf(1);
	Chip_GPIO_SetPinState(LPC_GPIO, _cs_gpio, _cs_pin, false);
	Chip_SSP_WriteFrames_Blocking(LPC_SSP1, Tx_Buf, 1);
	Chip_GPIO_SetPinState(LPC_GPIO, _cs_gpio, _cs_pin, true);
}

void LC1024_ReadMem(uint8_t *data, uint8_t len) {
    Tx_Buf[0] = READ_MEM_INSTR;
    Tx_Buf[1] = 0x0;
    Tx_Buf[2] = 0xF;
    Tx_Buf[3] = 0xF;
    ZeroTxBuf(4);
    xf_setup.length = 4+len; xf_setup.rx_cnt = 0; xf_setup.tx_cnt = 0;
    xf_setup.rx_data = data;

    Chip_GPIO_SetPinState(LPC_GPIO, _cs_gpio, _cs_pin, false);
    Chip_SSP_RWFrames_Blocking(LPC_SSP1, &xf_setup);
    Chip_GPIO_SetPinState(LPC_GPIO, _cs_gpio, _cs_pin, true);
}


void LC1024_WriteMem(void) {
    Tx_Buf[0] = WRITE_MEM_INSTR;
    Tx_Buf[1] = 0x0;
    Tx_Buf[2] = 0xF;
    Tx_Buf[3] = 0xF;
    Tx_Buf[4] = 0xA;
    Tx_Buf[5] = 0xA;
    ZeroTxBuf(6);

    Chip_GPIO_SetPinState(LPC_GPIO, _cs_gpio, _cs_pin, false);
	Chip_SSP_WriteFrames_Blocking(LPC_SSP1, Tx_Buf, 6);
    Chip_GPIO_SetPinState(LPC_GPIO, _cs_gpio, _cs_pin, true);
}

void LC1024_WriteEnable(void) {
    Tx_Buf[0] = WRITE_ENABLE_INSTR;
    ZeroTxBuf(1);
	Chip_GPIO_SetPinState(LPC_GPIO, _cs_gpio, _cs_pin, false);
	Chip_SSP_WriteFrames_Blocking(LPC_SSP1, Tx_Buf, 1);
	Chip_GPIO_SetPinState(LPC_GPIO, _cs_gpio, _cs_pin, true);
}
