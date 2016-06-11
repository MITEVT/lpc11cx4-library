#include "chip.h"
#include "mcp2515.h"
#include "util.h"
#include <string.h>

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/

#define TEST_CCAN_BAUD_RATE 500000

#define LED_PORT 0
#define LED_PIN 7

#define BUFFER_SIZE 8
#define UART_RX_BUFFER_SIZE 8
#define MCP_INT_PIN 0,11
#define MCP_CS_PIN 2,10
#define CLKOUT_DIV 1


const uint32_t OscRateIn = 12000000;


volatile uint32_t msTicks;

CCAN_MSG_OBJ_T temp_msg;
CCAN_MSG_OBJ_T send_msg;

static char uart_rx_buf[UART_RX_BUFFER_SIZE];


uint8_t tmp;

#define DEBUG_ENABLE

#ifdef DEBUG_ENABLE
	#define DEBUG_Print(str) Chip_UART_SendBlocking(LPC_USART, str, strlen(str))
	#define DEBUG_Write(str, count) Chip_UART_SendBlocking(LPC_USART, str, count)
#else
	#define DEBUG_Print(str)
	#define DEBUG_Write(str, count) 
#endif

/*****************************************************************************
 * Private functions
 ****************************************************************************/

void SysTick_Handler(void) {
	msTicks++;
}

void Board_UART_Print(const char *str) {
	Chip_UART_SendBlocking(LPC_USART, str, strlen(str));
}

void Board_UART_Println(const char *str) {
	Board_UART_Print(str);
	Board_UART_Print("\r\n");
}

void Board_UART_PrintNum(const int num, uint8_t base, bool crlf) {
	static char str[32];
	itoa(num, str, base);
	Board_UART_Print(str);
	if (crlf) Board_UART_Print("\r\n");
}

static void Init_SSP_PinMux(void){
	Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO0_8, (IOCON_FUNC1 | IOCON_MODE_INACT));
	Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO0_9, (IOCON_FUNC1 | IOCON_MODE_INACT));
	Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO2_11, (IOCON_FUNC1 | IOCON_MODE_INACT));
	Chip_IOCON_PinLocSel(LPC_IOCON,	IOCON_SCKLOC_PIO2_11);
}

void Board_MCP2515_Init(void){
	SSP_ConfigFormat ssp_format;
	Init_SSP_PinMux();
	Chip_SSP_Init(LPC_SSP0);
	Chip_SSP_SetBitRate(LPC_SSP0, 9600);

	ssp_format.frameFormat = SSP_FRAMEFORMAT_SPI;
	ssp_format.bits = SSP_BITS_8;
	ssp_format.clockMode = SSP_CLOCK_MODE0;
	Chip_SSP_SetFormat(LPC_SSP0, ssp_format.bits, ssp_format.frameFormat, ssp_format.clockMode);
	Chip_SSP_SetMaster(LPC_SSP0, true);
	Chip_SSP_Enable(LPC_SSP0);

	Chip_Clock_SetCLKOUTSource(SYSCTL_CLKOUTSRC_MAINSYSCLK, CLKOUT_DIV);
	Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO0_1, (IOCON_FUNC1 | IOCON_MODE_INACT));	/* CLKOUT */

	MCP2515_Init(MCP_CS_PIN, MCP_INT_PIN);
	MCP2515_SetBitRate(500, 12, 1);	

	MCP2515_BitModify(RXB0CTRL, RXM_MASK, RXM_OFF);
	MCP2515_BitModify(CANCTRL, MODE_MASK | CLKEN_MASK | CLKPRE_MASK, MODE_NORMAL | CLKEN_ENABLE | CLKPRE_CLKDIV_1);	
	MCP2515_Read(CANCTRL, &tmp, 1);
	Board_UART_PrintNum(tmp,2,true);


}

void Board_MCP2551_Check(uint8_t *tmp){
	MCP2515_Read(CANINTF, tmp, 1);
}

int main(void)
{

	SystemCoreClockUpdate();

	if (SysTick_Config (SystemCoreClock / 1000)) {
		//Error
		while(1);
	}

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

	DEBUG_Print("Started up\n\r");	
	Board_MCP2515_Init();
	uint8_t tmp;
	MCP2515_Read(CANINTE, &tmp, 1);
	Board_UART_PrintNum(tmp,2,true);
	MCP2515_Read(TXRTSCTRL,&tmp,1);
	Board_UART_PrintNum(tmp,2,true);

	while (1) {	
		Board_MCP2551_Check(&tmp);
		if (tmp & 1){
			MCP2515_ReadBuffer(&temp_msg, 0);
			Board_UART_Print("ID Received :");
			Board_UART_Print("0x");
			Board_UART_PrintNum(temp_msg.mode_id,16,true);

			MCP2515_Read(CANINTF,&tmp,1);
			MCP2515_Write(CANINTF,tmp & 0xFC);
		}

		MCP2515_Read(EFLG,&tmp,1);
		if(tmp){
			Board_UART_Print("CAN Error: 0b");
			Board_UART_PrintNum(tmp,2,true);
			MCP2515_Write(EFLG,0);
		}

		uint8_t count;
		if ((count = Chip_UART_Read(LPC_USART, uart_rx_buf, UART_RX_BUFFER_SIZE)) != 0) {
			switch (uart_rx_buf[0]) {
				case 'a':
					Board_UART_Println("Sending");
					send_msg.msgobj=2;
					send_msg.mode_id=0x601;
					send_msg.mask=0x7FF;
					send_msg.dlc = 1;
					send_msg.data[0] = 0xF0;
					MCP2515_LoadBuffer(0,&send_msg);
					MCP2515_SendBuffer(0);
					break;
				default:
					DEBUG_Print("Invalid Command\r\n");
					break;
			}
		}
	}
}
