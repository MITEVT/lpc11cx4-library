
#include "chip.h"
#include "mcp2515.h"
#include "util.h"

const uint32_t OscRateIn = 0; 

#define BUFFER_SIZE                         (0x100)
#define SSP_DATA_BITS                       (SSP_BITS_8)
#define LPC_SSP           LPC_SSP0
#define SSP_IRQ           SSP0_IRQn
#define SSPIRQHANDLER     SSP0_IRQHandler


#define CLKOUT_DIV 2

#define LED_GPIO 0
#define LED_PIN 7

#define CHIP_SELECT_GPIO 0
#define CHIP_SELECT_PIN 2

#define set_gpio_pin_output(gpio, pin) (Chip_GPIO_WriteDirBit(LPC_GPIO, gpio, pin, true))
#define set_gpio_pin_high(gpio, pin) (Chip_GPIO_SetPinState(LPC_GPIO, gpio, pin, true))
#define set_gpio_pin_low(gpio, pin) (Chip_GPIO_SetPinState(LPC_GPIO, gpio, pin, false))

/* Tx buffer */
static uint8_t Tx_Buf[BUFFER_SIZE];

/* Rx buffer */
static uint8_t Rx_Buf[BUFFER_SIZE];

static CCAN_MSG_OBJ_T msgobj;

static SSP_ConfigFormat ssp_format;
static Chip_SSP_DATA_SETUP_T xf_setup;
static volatile uint8_t  isXferCompleted = 0;
static char str[20];

static void Init_SSP_PinMux(void) {
	Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO0_8, (IOCON_FUNC1 | IOCON_MODE_INACT));	/* MISO0 */
	Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO0_9, (IOCON_FUNC1 | IOCON_MODE_INACT));	/* MOSI0 */
	Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO2_11, (IOCON_FUNC1 | IOCON_MODE_INACT));	/* SCK0 */
	Chip_IOCON_PinLocSel(LPC_IOCON, IOCON_SCKLOC_PIO2_11);
}

/**
 * @brief	Main routine for SSP example
 * @return	Nothing
 */
int main(void)
{
	SystemCoreClockUpdate();

	/* LED Initialization */
	Chip_GPIO_Init(LPC_GPIO);
	set_gpio_pin_output(LED_GPIO, LED_PIN);


	/* SSP initialization */
	Init_SSP_PinMux();
	Chip_SSP_Init(LPC_SSP);
	Chip_SSP_SetBitRate(LPC_SSP, 9600);

	ssp_format.frameFormat = SSP_FRAMEFORMAT_SPI;
	ssp_format.bits = SSP_DATA_BITS;
	ssp_format.clockMode = SSP_CLOCK_MODE0;
	Chip_SSP_SetFormat(LPC_SSP, ssp_format.bits, ssp_format.frameFormat, ssp_format.clockMode);
	Chip_SSP_SetMaster(LPC_SSP, true);
	Chip_SSP_Enable(LPC_SSP);

	Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO0_2, (IOCON_FUNC0 | IOCON_MODE_INACT));	/* PIO0_2 */
	Chip_GPIO_WriteDirBit(LPC_GPIO, 0, 2, true);

	// Set CLKOUT Source to be the Main Sys Clock divded by 1
	Chip_Clock_SetCLKOUTSource(SYSCTL_CLKOUTSRC_MAINSYSCLK, CLKOUT_DIV);
	Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO0_1, (IOCON_FUNC1 | IOCON_MODE_INACT));	/* CLKOUT */

	set_gpio_pin_high(LED_GPIO, LED_PIN);

	//---------------
	//UART
	Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO1_6, (IOCON_FUNC1 | IOCON_MODE_INACT));/* RXD */
	Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO1_7, (IOCON_FUNC1 | IOCON_MODE_INACT));/* TXD */

	Chip_UART_Init(LPC_USART);
	Chip_UART_SetBaud(LPC_USART, 9600);
	Chip_UART_ConfigData(LPC_USART, (UART_LCR_WLEN8 | UART_LCR_SBS_1BIT | UART_LCR_PARITY_DIS));
	Chip_UART_SetupFIFOS(LPC_USART, (UART_FCR_FIFO_EN | UART_FCR_TRG_LEV2));
	Chip_UART_TXEnable(LPC_USART);
	//---------------
	// MCP2515

	MCP2515_Init(CHIP_SELECT_GPIO, CHIP_SELECT_PIN);
	uint8_t i = MCP2515_SetBitRate(500, 11, 2);
	itoa(i, str, 10);
	if (i) {
		Chip_UART_SendBlocking(LPC_USART, "Baud Error: ", 12);
		Chip_UART_SendBlocking(LPC_USART, str, 2);
		Chip_UART_SendBlocking(LPC_USART, "\r\n", 2);
	}

	//---------------

	char *startMessage = "Started Up\n\r";
	Chip_UART_SendBlocking(LPC_USART, startMessage, 12);

	MCP2515_BitModify(CANCTRL, MODE_MASK | CLKEN_MASK | CLKPRE_MASK, MODE_LOOPBACK | CLKEN_ENABLE | CLKPRE_CLKDIV_1);

	MCP2515_BitModify(RXB0CTRL, RXM_MASK, RXM_OFF);

	msgobj.mode_id = 0x300;
	msgobj.dlc = 1;
	msgobj.data[0] = 0xAA;

	MCP2515_LoadBuffer(0, &msgobj);

	MCP2515_SendBuffer(0);

	msgobj.mode_id = 0;

	MCP2515_ReadBuffer(&msgobj, 0);

	if (msgobj.mode_id = 0x300) {
		Chip_UART_SendBlocking(LPC_USART, "Received Message\r\n", 18);
	}

	uint8_t tmp = 0;
	MCP2515_Read(CANINTF, &tmp, 1);
	uint8ToASCIIHex(tmp, str);
	Chip_UART_SendBlocking(LPC_USART, "CANINTF: 0x", 11);
	Chip_UART_SendBlocking(LPC_USART, str, 2);
	Chip_UART_SendBlocking(LPC_USART, "\n\r", 2);

	MCP2515_Read(EFLG, &tmp, 1);
	uint8ToASCIIHex(tmp, str);
	Chip_UART_SendBlocking(LPC_USART, "EFLG: 0x", 8);
	Chip_UART_SendBlocking(LPC_USART, str, 2);
	Chip_UART_SendBlocking(LPC_USART, "\n\r", 2);

	MCP2515_Read(TEC, &tmp, 1);
	itoa(tmp, str, 10);
	Chip_UART_SendBlocking(LPC_USART, "TEC: ", 5);
	Chip_UART_SendBlocking(LPC_USART, str, 4);
	Chip_UART_SendBlocking(LPC_USART, "\n\r", 2);
 

	// Chip_GPIO_SetPinState(LPC_GPIO, CHIP_SELECT_GPIO, CHIP_SELECT_PIN, false);
	// Chip_SSP_WriteFrames_Blocking(LPC_SSP, Tx_Buf, 3);
	// Chip_GPIO_SetPinState(LPC_GPIO, CHIP_SELECT_GPIO, CHIP_SELECT_PIN, true);

	// bool rece = false;	
	// uint8_t stat = 0;

	// MCP2515_Write(TXB0SIDH, 0xAB);

	while (1) {

	}
	return 0;
}
