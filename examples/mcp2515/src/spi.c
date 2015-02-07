
#include "chip.h"

const uint32_t OscRateIn = 12000000;

#define SSP_MODE_TEST       1	/*1: Master, 0: Slave */
#define BUFFER_SIZE                         (0x100)
#define SSP_DATA_BITS                       (SSP_BITS_8)
#define SSP_DATA_BIT_NUM(databits)          (databits + 1)
#define SSP_DATA_BYTES(databits)            (((databits) > SSP_BITS_8) ? 2 : 1)
#define SSP_LO_BYTE_MSK(databits)           ((SSP_DATA_BYTES(databits) > 1) ? 0xFF : (0xFF >> \
																					  (8 - SSP_DATA_BIT_NUM(databits))))
#define SSP_HI_BYTE_MSK(databits)           ((SSP_DATA_BYTES(databits) > 1) ? (0xFF >> \
																			   (16 - SSP_DATA_BIT_NUM(databits))) : 0)
#define LPC_SSP           LPC_SSP0
#define SSP_IRQ           SSP0_IRQn
#define SSPIRQHANDLER     SSP0_IRQHandler


#define LED_PIN 7
/* Tx buffer */
static uint8_t Tx_Buf[BUFFER_SIZE];

/* Rx buffer */
static uint8_t Rx_Buf[BUFFER_SIZE];

static SSP_ConfigFormat ssp_format;
static Chip_SSP_DATA_SETUP_T xf_setup;
static volatile uint8_t  isXferCompleted = 0;

static void Init_SSP_PinMux(void) {
	Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO0_8, (IOCON_FUNC1 | IOCON_MODE_INACT));	/* MISO0 */
	Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO0_9, (IOCON_FUNC1 | IOCON_MODE_INACT));	/* MOSI0 */
	Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO0_2, (IOCON_FUNC1 | IOCON_MODE_INACT));	/* SSEL0 */
	Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO2_11, (IOCON_FUNC1 | IOCON_MODE_INACT));	/* SCK0 */
	Chip_IOCON_PinLocSel(LPC_IOCON, IOCON_SCKLOC_PIO2_11);
}

static void Buffer_Init(void)
{
	uint16_t i;
	uint8_t ch = 0;

	for (i = 0; i < BUFFER_SIZE; i++) {
		Tx_Buf[i] = 0x01;
		Rx_Buf[i] = 0xAA;
	}
}

static void GPIO_Config(void) {
	Chip_GPIO_Init(LPC_GPIO);

}

static void LED_Config(void) {
	Chip_GPIO_WriteDirBit(LPC_GPIO, 0, LED_PIN, true);

}

static void LED_On(void) {
	Chip_GPIO_SetPinState(LPC_GPIO, 0, LED_PIN, true);
}

static void LED_Off(void) {
	Chip_GPIO_SetPinState(LPC_GPIO, 0, LED_PIN, false);
}

/**
 * @brief	Main routine for SSP example
 * @return	Nothing
 */
int main(void)
{
	SystemCoreClockUpdate();

	/* LED Initialization */
	GPIO_Config();
	LED_Config();


	/* SSP initialization */
	Init_SSP_PinMux();
	Chip_SSP_Init(LPC_SSP);
	Chip_SSP_SetBitRate(LPC_SSP, 460800);

	ssp_format.frameFormat = SSP_FRAMEFORMAT_SPI;
	ssp_format.bits = SSP_DATA_BITS;
	ssp_format.clockMode = SSP_CLOCK_MODE0;
	Chip_SSP_SetFormat(LPC_SSP, ssp_format.bits, ssp_format.frameFormat, ssp_format.clockMode);
	Chip_SSP_SetMaster(LPC_SSP, SSP_MODE_TEST);
	Chip_SSP_Enable(LPC_SSP);


	Buffer_Init();

	Chip_Clock_SetCLKOUTSource(SYSCTL_CLKOUTSRC_MAINSYSCLK, 1);
	Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO0_1, (IOCON_FUNC1 | IOCON_MODE_INACT));	/* MISO0 */

	LED_On();

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

	char *startMessage = "Started Up\n\r";
	Chip_UART_SendBlocking(LPC_USART, startMessage, 12);


	xf_setup.tx_data = Tx_Buf;
	xf_setup.rx_data = Rx_Buf;


	int i;

	// Send RESET Signal to MCP2515
	Tx_Buf[0] = 0xC0; // Reset Command

	Chip_SSP_WriteFrames_Blocking(LPC_SSP, Tx_Buf, 1);

	Tx_Buf[1] = 0x02; // Write Command
	Tx_Buf[1] = 0x28; // Address of CNF3
	Tx_Buf[2] = 0x07; // PS2 = 8*Tq
	Tx_Buf[3] = 0xBE; // PS1 = 8*Tq, PropSeg=7*Tq
	Tx_Buf[4] = 0x80; // SJW=3*Tq, BRP = 0

	Chip_SSP_WriteFrames_Blocking(LPC_SSP, Tx_Buf, 5);

	Tx_Buf[0] = 0x05; //Bit Modify Command
	Tx_Buf[1] = 0x0F; //Address of CANCTRL
	Tx_Buf[2] = 0xE0; //Only set Top Three Bits
	Tx_Buf[3] = 0x00; //Send to Normal Mode

	Chip_SSP_WriteFrames_Blocking(LPC_SSP, Tx_Buf, 4);



	while (1) {
		Tx_Buf[0] = 0xC0; // Write Command
		Tx_Buf[1] = 0x31; // Address of TXB0SIDH
		Tx_Buf[2] = 0x03; // TXB0SIDH
		Tx_Buf[3] = 0x00; // TXB0SIDL
		Tx_Buf[4] = 0x00; // TXB0EID8
		Tx_Buf[5] = 0x00; // TXB0EID0
		Tx_Buf[6] = 0x01; // TXB0DLC
		Tx_Buf[7] = 0xAA;

		Chip_SSP_WriteFrames_Blocking(LPC_SSP, Tx_Buf, 8);

		Tx_Buf[0] = 0x05;
		Tx_Buf[1] = 0x30;
		Tx_Buf[2] = 0x08;
		Tx_Buf[3] = 0x80;

		Chip_SSP_WriteFrames_Blocking(LPC_SSP, Tx_Buf, 4);


		Tx_Buf[0] = 0x03;

		Chip_SSP_WriteFrames_Blocking(LPC_SSP, Tx_Buf, 1);

		Tx_Buf[0] = 0x31;
		Tx_Buf[1] = 0x00;
		xf_setup.length = 8;
		xf_setup.rx_cnt = 0;
		xf_setup.tx_cnt = 0;

		Chip_SSP_Int_RWFrames8Bits(LPC_SSP, &xf_setup);

		if (xf_setup.rx_cnt != 0) {
			Chip_UART_SendBlocking(LPC_USART, Rx_Buf, 1);
		} else {
			Chip_UART_SendBlocking(LPC_USART, "None\n\r", 6);
		}


	}
	return 0;
}
