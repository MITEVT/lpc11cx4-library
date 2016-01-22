/**************************
* 	I2C Master Example    *
***************************/


#include "chip.h"
#include "util.h"
#include <string.h>

const uint32_t OscRateIn = 0000000;

#define LED0 2, 5
#define LED1 0, 6
#define LED2 0, 7
#define LED3 2, 9

#define SCL 0, 4
#define SDA 0, 5

#define SLAVEADDR 0x64

#define print(str) {Chip_UART_SendBlocking(LPC_USART, str, strlen(str));}
#define println(str) {print(str); print("\r\n");}
#define printnum(val, buf, base) {itoa(val, buf, base); print(buf);}

volatile uint32_t msTicks;

uint8_t rx_buf[8];
char print_buf[32];
char cmd_buf[20];
uint8_t cmd_count = 0;

void SysTick_Handler(void) {
	msTicks++;
}


int main(void)
{

	SystemCoreClockUpdate();

	if (SysTick_Config (SystemCoreClock / 1000)) {
		//Error
		while(1);
	}

	// Initialize UART

	Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO1_6, (IOCON_FUNC1 | IOCON_MODE_INACT));/* RXD */
	Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO1_7, (IOCON_FUNC1 | IOCON_MODE_INACT));/* TXD */

	Chip_UART_Init(LPC_USART);
	Chip_UART_SetBaud(LPC_USART, 57600);
	Chip_UART_ConfigData(LPC_USART, (UART_LCR_WLEN8 | UART_LCR_SBS_1BIT | UART_LCR_PARITY_DIS));
	Chip_UART_SetupFIFOS(LPC_USART, (UART_FCR_FIFO_EN | UART_FCR_TRG_LEV2));
	Chip_UART_TXEnable(LPC_USART);

	cmd_count = 0;
	cmd_buf[0] = '\0';

	// Turn on Power LED

	Chip_GPIO_Init(LPC_GPIO);
	Chip_GPIO_WriteDirBit(LPC_GPIO, LED0, 1);
	Chip_GPIO_WriteDirBit(LPC_GPIO, LED1, 1);
	Chip_GPIO_SetPinState(LPC_GPIO, LED0, 1);

	// Initialize I2C

	Chip_SYSCTL_PeriphReset(RESET_I2C0);
	Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO0_4, IOCON_FUNC1); // SCL
	Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO0_5, IOCON_FUNC1); // SDA

	Chip_I2C_Init(I2C0);
	// Chip_I2C_SetClockRate(I2C0, 100000);

	// Chip_I2C_SetMasterEventHandler(I2C0, Chip_I2C_EventHandler);
	// NVIC_EnableIRQ(I2C0_IRQn);


	uint32_t heartbeat_ticks = msTicks;
	uint8_t heartbeat_flag = 0;

	// End of Initialization
	Chip_GPIO_SetPinState(LPC_GPIO, LED1, 1);
	print(">");

	// Main loop
    while(1) {
		uint8_t count;
		if ((count = Chip_UART_Read(LPC_USART, rx_buf, 8)) != 0) {
			Chip_UART_SendBlocking(LPC_USART, rx_buf, count);
			if (rx_buf[0] == '\r' || rx_buf[0] == '\n') {
				if (cmd_buf[0] == 'h') {
					print("h - Help Prompt.");
				} else if (cmd_buf[0] == 's') {
				} else if (cmd_buf[0] == 'a') {
				} else if (cmd_buf[0] == '\0') {
				} else {
					print("Unknown command.");
				}
				println("");
				print(">");
				cmd_buf[0] = '\0';
				cmd_count = 0;
			} else {
				memcpy(cmd_buf + cmd_count, rx_buf, count);
			}
			
        
		}

		if (msTicks - heartbeat_ticks > 500) {
			heartbeat_ticks = msTicks;
			heartbeat_flag = 1 - heartbeat_flag;
			Chip_GPIO_SetPinState(LPC_GPIO, LED0, heartbeat_flag);
		}
	}

	return 0;
}
