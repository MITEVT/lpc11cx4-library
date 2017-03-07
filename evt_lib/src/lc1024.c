#include "lc1024.h"
#include <string.h>

#define SPI_BUFFER_SIZE 80

static uint8_t Tx_Buf[SPI_BUFFER_SIZE];
static Chip_SSP_DATA_SETUP_T xf_setup;
static uint8_t _cs_gpio;
static uint8_t _cs_pin;
static uint32_t _baud;
static LPC_SSP_T *_pSSP;

static char str[10];


void ZeroTxBuf(uint8_t start);

void LC1024_Init(LPC_SSP_T *pSSP, uint32_t baud, uint8_t cs_gpio, uint8_t cs_pin) {
    _pSSP = pSSP;
	_baud = baud;
	_cs_gpio = cs_gpio;
	_cs_pin = cs_pin;

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
	Chip_SSP_RWFrames_Blocking(_pSSP, &xf_setup);
	Chip_GPIO_SetPinState(LPC_GPIO, _cs_gpio, _cs_pin, true);
}

void LC1024_WriteDisable(void) {
    Tx_Buf[0] = WRITE_DISABLE_INSTR;
    ZeroTxBuf(1);
	Chip_GPIO_SetPinState(LPC_GPIO, _cs_gpio, _cs_pin, false);
	Chip_SSP_WriteFrames_Blocking(_pSSP, Tx_Buf, 1);
	Chip_GPIO_SetPinState(LPC_GPIO, _cs_gpio, _cs_pin, true);
}

void LC1024_ReadMem(uint8_t *address, uint8_t *rcv_buf, uint8_t len) {
    Tx_Buf[0] = READ_MEM_INSTR;
    Tx_Buf[1] = address[0];
    Tx_Buf[2] = address[1];
    Tx_Buf[3] = address[2];
    ZeroTxBuf(4);
    xf_setup.length = 4+len; xf_setup.rx_cnt = 0; xf_setup.tx_cnt = 0;
    xf_setup.rx_data = rcv_buf;

    Chip_GPIO_SetPinState(LPC_GPIO, _cs_gpio, _cs_pin, false);
    Chip_SSP_RWFrames_Blocking(_pSSP, &xf_setup);
    Chip_GPIO_SetPinState(LPC_GPIO, _cs_gpio, _cs_pin, true);

	memcpy(rcv_buf, rcv_buf+4, len);
}


void LC1024_WriteMem(uint8_t *address, uint8_t *data, uint8_t len) {

    LC1024_WriteEnable();
    
    Tx_Buf[0] = WRITE_MEM_INSTR;
    Tx_Buf[1] = address[0];
    Tx_Buf[2] = address[1];
    Tx_Buf[3] = address[2];
    uint8_t i;
    for(i = 0; i < len; i++) {
        Tx_Buf[4 + i] = data[i];
    }
    ZeroTxBuf(4+len);

    Chip_GPIO_SetPinState(LPC_GPIO, _cs_gpio, _cs_pin, false);
	Chip_SSP_WriteFrames_Blocking(_pSSP, Tx_Buf, 4+len);
    Chip_GPIO_SetPinState(LPC_GPIO, _cs_gpio, _cs_pin, true);
}

void LC1024_WriteEnable(void) {
    Tx_Buf[0] = WRITE_ENABLE_INSTR;
    ZeroTxBuf(1);
	Chip_GPIO_SetPinState(LPC_GPIO, _cs_gpio, _cs_pin, false);
	Chip_SSP_WriteFrames_Blocking(_pSSP, Tx_Buf, 1);
	Chip_GPIO_SetPinState(LPC_GPIO, _cs_gpio, _cs_pin, true);
}
