#include "chip.h"
#include "mcp2515.h"
#include "util.h"
// #include "board.h"

#define BUFFER_SIZE                         (256)

#define set_gpio_pin_output(gpio, pin) (Chip_GPIO_WriteDirBit(LPC_GPIO, gpio, pin, true))
#define set_gpio_pin_input(gpio, pin) (Chip_GPIO_WriteDirBit(LPC_GPIO, gpio, pin, false))
#define set_gpio_pin_high(gpio, pin) (Chip_GPIO_SetPinState(LPC_GPIO, gpio, pin, true))
#define set_gpio_pin_low(gpio, pin) (Chip_GPIO_SetPinState(LPC_GPIO, gpio, pin, false))


/* Tx buffer */
static uint8_t Tx_Buf[BUFFER_SIZE];

/* Rx buffer */
static uint8_t Rx_Buf[BUFFER_SIZE];

static Chip_SSP_DATA_SETUP_T xf_setup;

uint8_t _cs_gpio;
uint8_t _cs_pin;
uint8_t _int_gpio;
uint8_t _int_pin;

void MCP2515_Init(uint8_t cs_gpio, uint8_t cs_pin, uint8_t int_gpio, uint8_t int_pin) {
	_cs_gpio = cs_gpio;
	_cs_pin = cs_pin;

	_int_gpio = int_gpio;
	_int_pin = int_pin;

	set_gpio_pin_output(_cs_gpio, _cs_pin);
	set_gpio_pin_input(_int_gpio, _int_pin);

	xf_setup.tx_data = Tx_Buf;
	xf_setup.rx_data = Rx_Buf;
}


// Baud in KHz, Freq in Mhz
uint8_t MCP2515_SetBitRate(uint32_t baud, uint32_t freq, uint8_t SJW) {
	if (SJW < 1) SJW = 1;
	if (SJW > 4) SJW = 4;
	if (baud > 0) {
		MCP2515_Reset();

		uint8_t BRP;
		uint64_t TQ;
		uint8_t BT = 0;
		uint64_t tempBT;

		uint64_t NBT = 10000 * 1 / baud * 1000; 		// Since baud wil probably not go above 10 Mhz, 
														// this will prevent rounding losses 
		for (BRP = 0; BRP < 64; BRP++) {
			TQ = 10000 * 2 * (BRP + 1) / freq;
			tempBT = NBT / TQ; 							// The added 10K should cancel out here
			if (tempBT <= 25) {
				BT = (int) tempBT;
				if (tempBT - BT == 0) break;
			}
		}

		uint8_t SPT = (0.7 * BT);
		uint8_t PRSEG = (SPT - 1) / 2;
		uint8_t PHSEG1 = SPT - PRSEG - 1;
		uint8_t PHSEG2 = BT - PHSEG1 - PRSEG - 1;


		if (PRSEG + PHSEG1 < PHSEG2) {
			return 1;
		}

		if (PHSEG2 <= SJW) {
			return 2;
		}


		uint8_t BTLMODE = 1;
		uint8_t SAM = 0;

		uint8_t data = (((SJW - 1) << 6) | BRP);

		MCP2515_Write(CNF1, data);
		MCP2515_Write(CNF2, ((BTLMODE << 7) | (SAM << 6) | ((PHSEG1 - 1) << 3) | (PRSEG - 1)));
		MCP2515_Write(CNF3, (0x00 | (PHSEG2 - 1)));
		MCP2515_Write(TXRTSCTRL, 0);

		// if (!MCP2515_Mode(MODE_NORMAL)) {
		// 	return 3;
		// } 

		// Enable interrupts: Receive Buffers Full
		MCP2515_Write(CANINTE, 0x03);
		// MCP2515_Write(CANINTE, 0x00);

		uint8_t rtn = 0;
		MCP2515_Read(CNF1, &rtn, 1);
		if (rtn == data) {
			return 0;
		} else {
			return 4;
		}
	} else {
		return 5;
	}

	return 0;
}

void MCP2515_Reset(void) {
	set_gpio_pin_low(_cs_gpio, _cs_pin);
	Tx_Buf[0] = MCP2515_RESET; // Reset Command
	Chip_SSP_WriteFrames_Blocking(LPC_SSP, Tx_Buf, 1);
	set_gpio_pin_high(_cs_gpio, _cs_pin);
}

int8_t MCP2515_GetFullReceiveBuffer(void) {
	if (Chip_GPIO_GetPinState(LPC_GPIO, _int_gpio, _int_pin))
		return -1;
	uint8_t tmp;
	MCP2515_Read(CANINTF, &tmp, 1);
	if (tmp & 0x3) {
		MCP2515_BitModify(CANINTF, 0x03, 0x00);
		return 2;
	} else if (tmp & 0x1) {
		MCP2515_BitModify(CANINTF, 0x01, 0x00);
		return 0;
	} else if (tmp & 0x2) {
		MCP2515_BitModify(CANINTF, 0x02, 0x00);
		return 1;
	} else {
		return -1;
	}
}

int8_t MCP2515_GetEmptyTransmitBuffer(void) {
	if (Chip_GPIO_GetPinState(LPC_GPIO, _int_gpio, _int_pin))
		return -1;
	uint8_t tmp;
	MCP2515_Read(CANINTF, &tmp, 1);
	if (tmp & 0x4) {
		MCP2515_BitModify(CANINTF, 0x04, 0x00);
		return 0;
	} else if (tmp & 0x8) {
		MCP2515_BitModify(CANINTF, 0x08, 0x00);
		return 1;
	} else if (tmp & 0x10) { 
		MCP2515_BitModify(CANINTF, 0x10, 0x00);
		return 2;
	} else {
		return -1;
	}
}

void MCP2515_Read(uint8_t address, uint8_t *buf, uint8_t length) {
	uint8_t i;

	set_gpio_pin_low(_cs_gpio, _cs_pin);
	Tx_Buf[0] = MCP2515_READ;
	Tx_Buf[1] = address;
	xf_setup.length = 2+length;
	xf_setup.rx_cnt = 0;
	xf_setup.tx_cnt = 0;
	// DEBUG_Println("Reading SSP");
	Chip_SSP_RWFrames_Blocking(LPC_SSP, &xf_setup);
	// itoa(xf_setup.rx_cnt, str, 10);
	// DEBUG_Print("Received: ");
	// DEBUG_Println(str);
	for (i = 0; i < length; i++) {
		buf[i] = Rx_Buf[i + 2];
	}

	set_gpio_pin_high(_cs_gpio, _cs_pin);
}

void MCP2515_ReadBuffer(CCAN_MSG_OBJ_T *msgobj, uint8_t bufferNum) {
	set_gpio_pin_low(_cs_gpio, _cs_pin);
	Tx_Buf[0] = MCP2515_READ_RX_BUF(bufferNum);
	xf_setup.length = 14;
	xf_setup.rx_cnt = 0;
	xf_setup.tx_cnt = 0;
	Chip_SSP_RWFrames_Blocking(LPC_SSP, &xf_setup);
	msgobj->mode_id = getIDFromBytes(Rx_Buf[1], Rx_Buf[2]);

	msgobj->dlc = Rx_Buf[5] & 0x0F;
	if (msgobj->dlc > 8)
		msgobj->dlc = 8;

	uint8_t i;
	for (i = 0; i < msgobj->dlc; i++) {
		msgobj->data[i] = Rx_Buf[6+i];
	}

	set_gpio_pin_high(_cs_gpio, _cs_pin);
}

void MCP2515_Write(uint8_t address, uint8_t data) {
	set_gpio_pin_low(_cs_gpio, _cs_pin);
	Tx_Buf[0] = MCP2515_WRITE;
	Tx_Buf[1] = address;
	Tx_Buf[2] = data;
	Chip_SSP_WriteFrames_Blocking(LPC_SSP, Tx_Buf, 3);
	set_gpio_pin_high(_cs_gpio, _cs_pin);
}

void MCP2515_LoadBuffer(uint8_t buffer, CCAN_MSG_OBJ_T *msgobj) {
	Tx_Buf[0] = MCP2515_LOAD_TX_BUF(buffer);
	Tx_Buf[1] = (msgobj->mode_id >> 3) & 0xFF;
	Tx_Buf[2] = (msgobj->mode_id & 0x7) << 5;
	Tx_Buf[3] = 0;
	Tx_Buf[4] = 0;
	Tx_Buf[5] = msgobj->dlc;

	uint8_t i;
	for (i = 0 ; i < msgobj->dlc; i++) {
		Tx_Buf[6 + i] = msgobj->data[i];
	}

	set_gpio_pin_low(_cs_gpio, _cs_pin);
	Chip_SSP_WriteFrames_Blocking(LPC_SSP, Tx_Buf, 6 + msgobj->dlc);
	set_gpio_pin_high(_cs_gpio, _cs_pin);
}

void MCP2515_LoadBufferData(uint8_t buffer, CCAN_MSG_OBJ_T *msgobj) {
	Tx_Buf[0] = MCP2515_LOAD_TX_BUF(buffer) & 0x1;

	uint8_t i;
	for (i = 0 ; i < msgobj->dlc; i++) {
		Tx_Buf[1 + i] = msgobj->data[i];
	}

	set_gpio_pin_low(_cs_gpio, _cs_pin);
	Chip_SSP_WriteFrames_Blocking(LPC_SSP, Tx_Buf, 1 + msgobj->dlc);
	set_gpio_pin_high(_cs_gpio, _cs_pin);
}

void MCP2515_SendBuffer(uint8_t buffer) {
	set_gpio_pin_low(_cs_gpio, _cs_pin);
	switch (buffer) {
		case 0:
			Tx_Buf[0] = MCP2515_RTS0;
			break;
		case 1:
			Tx_Buf[0] = MCP2515_RTS1;
			break;
		case 2:
			Tx_Buf[0] = MCP2515_RTS2;
			break;
	}
	Chip_SSP_WriteFrames_Blocking(LPC_SSP, Tx_Buf, 1);
	set_gpio_pin_high(_cs_gpio, _cs_pin);
}

void MCP2515_BitModify(uint8_t address, uint8_t mask, uint8_t data) {
	set_gpio_pin_low(_cs_gpio, _cs_pin);
	Tx_Buf[0] = MCP2515_BIT_MODIFY;
	Tx_Buf[1] = address;
	Tx_Buf[2] = mask;
	Tx_Buf[3] = data;
	Chip_SSP_WriteFrames_Blocking(LPC_SSP, Tx_Buf, 4);
	set_gpio_pin_high(_cs_gpio, _cs_pin);
}

bool MCP2515_Mode(uint8_t mode) {
	MCP2515_BitModify(CANCTRL, MODE_MASK, mode);
	uint8_t data = 0;
	MCP2515_Read(CANSTAT, &data, 1);
	return ((data & mode) == mode);
}