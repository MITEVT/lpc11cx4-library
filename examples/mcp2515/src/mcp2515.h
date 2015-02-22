#define LPC_SSP           LPC_SSP0

#define CANSTAT 0x0E
#define CANCTRL 0x0F

#define TXRTSCTRL 0x0D

#define CANINTE 0x2B
#define CANINTF 0x2C

#define MCP2515_RESET 0xC0
#define MCP2515_READ  0x03
#define MCP2515_READ_RX_BUF(buf) (0x90 | (buf << 1))
#define MCP2515_WRITE 0x02
#define MCP2515_LOAD_TX_BUF(buf) (0x40 | buf)
#define MCP2515_RTS0  0x81
#define MCP2515_RTS1  0x82
#define MCP2515_RTS2  0x84
#define MCP2515_READ_STATUS 0xA0
#define MCP2515_RX_STATUS 0x0B
#define MCP2515_BIT_MODIFY 0x05

/* ===================== CANCTRL =================== */

#define MODE_MASK   0xE0
#define MODE_NORMAL 0x00
#define MODE_SLEEP  0x20
#define MODE_LOOPBACK 0x40
#define MODE_LISTEN 0x60
#define MODE_CONFIG 0x80

#define CLKEN_MASK  0x04
#define CLKEN_ENABLE 0x04
#define CLKEN_DISABLE 0x00

#define CLKPRE_MASK 0x03
#define CLKPRE_CLKDIV_1 0x00
#define CLKPRE_CLKDIV_2 0x01
#define CLKPRE_CLKDIV_4 0x10
#define CLKPRE_CLKDIV_8 0x11


/* ===================== CONFIGURATION =================== */

#define CNF3 0x28
#define SOF 7 		// Start-of-Frame Signal Bit
#define WAKFIL 6 	// Wake=up Filter Bit
#define PHSEG22 2 	// PS2 Length Bits
#define PHSEG21 1
#define PHSEG20 0

#define CNF2 0x29
// #define BTLMODE 7 	// PS2 Bit Time Length Bit
// #define SAM 6 		// Sample Point Configuration Bit
// #define PHSEG12 5 	// PS1 Length Bits
// #define PHSEG11 4
// #define PHSEG10 3
// #define PRSEG2  2 	// Propogation Segment Length Bits
// #define PRSEG1  1
// #define PRSEG0  0

#define CNF1 0x2A
#define SJW1 7 		// Synchronization Jump Width Bits
#define SJW0 6
#define BRP5 5 		// Baud Rate Prescalar bits
#define BRP4 4
#define BRP3 3
#define BRP2 2
#define BRP1 1
#define BRP0 0

#define BFPCTRL   0x0C
#define B1BFS 5 	// RX1BF Pin State
#define B0BFS 4 	// RX0BF Pin State
#define B1BFE 3		// RX1BF Pin Function Enable
#define B0BFE 2 	// RX0BF Pin Function Enable
#define B1BFM 1 	// RX1BF Pin Operation Mode
#define B0BFM 0 	// RX0BF Pin Operation Mode

/* =================== FILTERS AND MASKS ================= */

#define RXF0SIDH 0x00
#define RXF0SIDL 0x01
#define RXF0EID8 0x02
#define RXF0EID0 0x03
#define RXF1SIDH 0x04
#define RXF1SIDL 0x05
#define RXF1EID8 0x06
#define RXF1EID0 0x07
#define RXF2SIDH 0x08
#define RXF2SIDL 0x09
#define RXF2EID8 0x0A
#define RXF2EID0 0x0B
#define RXF3SIDH 0x10
#define RXF3SIDL 0x11
#define RXF3EID8 0x12
#define RXF3EID0 0x13
#define RXF4SIDH 0x14
#define RXF4SIDL 0x15
#define RXF4EID8 0x16
#define RXF4EID0 0x17
#define RXF5SIDH 0x18
#define RXF5SIDL 0x19
#define RXF5EID8 0x1A
#define RXF5EID0 0x1B

#define RXM0SIDH 0x20
#define RXM0SIDL 0x21
#define RXM0EID8 0x22
#define RXM0EID0 0x23
#define RXM1SIDH 0x24
#define RXM1SIDL 0x25
#define RXM1EID8 0x26
#define RXM1EID0 0x27

/* =================== TRANSMIT BUFFERS ================= */

#define TXB0CTRL 0x30
#define TXB0SIDH 0x31
#define TXB0SIDL 0x32
#define TXB0EID8 0x33
#define TXB0EID0 0x34
#define TXB0DLC  0x35
#define TXB0D0   0x36
#define TXB0D1   0x37
#define TXB0D2   0x38
#define TXB0D3   0x39
#define TXB0D4   0x3A
#define TXB0D5   0x3B
#define TXB0D6   0x3C
#define TXB0D7   0x3D

#define TXB1CTRL 0x40
#define TXB1SIDH 0x41
#define TXB1SIDL 0x42
#define TXB1EID8 0x43
#define TXB1EID0 0x44
#define TXB1DLC  0x45
#define TXB1D0   0x46
#define TXB1D1   0x47
#define TXB1D2   0x48
#define TXB1D3   0x49
#define TXB1D4   0x4A
#define TXB1D5   0x4B
#define TXB1D6   0x4C
#define TXB1D7   0x4D

#define TXB2CTRL 0x50
#define TXB2SIDH 0x51
#define TXB2SIDL 0x52
#define TXB2EID8 0x53
#define TXB2EID0 0x54
#define TXB2DLC  0x55
#define TXB2D0   0x56
#define TXB2D1   0x57
#define TXB2D2   0x58
#define TXB2D3   0x59
#define TXB2D4   0x5A
#define TXB2D5   0x5B
#define TXB2D6   0x5C
#define TXB2D7   0x5D

/* =================== RECEIVE BUFFERS ================= */


#define RXB0CTRL 0x60
#define RXB0SIDH 0x61
#define RXB0SIDL 0x62
#define RXB0EID8 0x63
#define RXB0EID0 0x64
#define RXB0DLC  0x65
#define RXB0D0   0x66
#define RXB0D1   0x67
#define RXB0D2   0x68
#define RXB0D3   0x69
#define RXB0D4   0x6A
#define RXB0D5   0x6B
#define RXB0D6   0x6C
#define RXB0D7   0x6D

#define RXB1CTRL 0x70
#define RXB1SIDH 0x71
#define RXB1SIDL 0x72
#define RXB1EID8 0x73
#define RXB1EID0 0x74
#define RXB1DLC  0x75
#define RXB1D0   0x76
#define RXB1D1   0x77
#define RXB1D2   0x78
#define RXB1D3   0x79
#define RXB1D4   0x7A
#define RXB1D5   0x7B
#define RXB1D6   0x7C
#define RXB1D7   0x7D

#define RXM_MASK 0x60
#define RXM_OFF  0x60
#define RXM_EXT  0x40
#define RXM_STD  0x20
#define RXM_ANY  0x00

/* =================== ERROR DETECTION ================= */

#define TEC 0x1C
#define REC 0x1D

#define EFLG	0x2D
#define EWARN 	0 		//Error Warning Flag Bit
#define RXWAR	1 		// Receive Error Warning Flag Bit
#define TXWAR 	2 		// Transmit Error Warning Flag Bit
#define RXEP 	3 		// Receive Error-Passive Flag Bit
#define TXEP 	4 		// Transmit Error-Passive Flag Bit
#define TXBO 	5 		// Bus-Off Error Flag Bit
#define RX0OVR 	6 		// Receive Buffer 0 Overflow Flag Bit
#define RX1OVR	7 		// Receive Buffer 1 Overflow Flag Bit


/* =================== FUNCTION PROTOS ================== */

void MCP2515_Init(uint8_t cs_gpio, uint8_t cs_pin);

uint8_t MCP2515_SetBitRate(uint32_t baud, uint32_t freq, uint8_t SJW);

void MCP2515_Reset(void);

void MCP2515_Read(uint8_t address, uint8_t *buf, uint8_t length);

void MCP2515_ReadBuffer(CCAN_MSG_OBJ_T *msgobj, uint8_t bufferNum);

void MCP2515_Write(uint8_t address, uint8_t data);

void MCP2515_LoadBuffer(uint8_t buffer, CCAN_MSG_OBJ_T *msgobj);

void MCP2515_SendBuffer(uint8_t buffer);

void MCP2515_BitModify(uint8_t address, uint8_t mask, uint8_t data);

bool MCP2515_Mode(uint8_t mode);
