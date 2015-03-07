/**************************
* 	Reads Config Input    *
* 	for Battery System    *
***************************/

struct Config {
	uint8_t Series;
	uint8_t Parallel;
	uint8_t ModularSeries;
	uint8_t ModularParallel;
};

typedef struct Config Config_t;

#include "chip.h"
#include <string.h>

const uint32_t OscRateIn = 0000000;

#define LED_PIN 7
#define HELP_CHAR1 '?'
#define HELP_CHAR1 'h'
#define READ_CHAR 'r'
#define SET_CHAR 's'


volatile uint32_t msTicks;

void SysTick_Handler(void) {
	msTicks++;
}

__INLINE static void Delay(uint32_t dlyTicks) {
	uint32_t curTicks = msTicks;
	while ((msTicks - curTicks) < dlyTicks);
}

__INLINE static void GPIO_Config(void) {
	Chip_GPIO_Init(LPC_GPIO);

}

__INLINE static void LED_Config(void) {
	Chip_GPIO_WriteDirBit(LPC_GPIO, 0, LED_PIN, true);

}

__INLINE static void LED_On(void) {
	Chip_GPIO_SetPinState(LPC_GPIO, 0, LED_PIN, true);
}

__INLINE static void LED_Off(void) {
	Chip_GPIO_SetPinState(LPC_GPIO, 0, LED_PIN, false);
}

void print_help(void) {
	const char *text = "Press 'r' to read the current battery settings.\n"
						"Press 's' to set the current battery settings.\n"
						"Press '?' or 'h' to repeat these instructions.\n\r";
	Chip_UART_SendBlocking(LPC_USART, text, strlen(text));
}

void print_config(Config_t *config) {

}

int main(void)
{

	SystemCoreClockUpdate();

	Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO1_6, (IOCON_FUNC1 | IOCON_MODE_INACT));/* RXD */
	Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO1_7, (IOCON_FUNC1 | IOCON_MODE_INACT));/* TXD */

	Chip_UART_Init(LPC_USART);
	Chip_UART_SetBaud(LPC_USART, 9600);
	Chip_UART_ConfigData(LPC_USART, (UART_LCR_WLEN8 | UART_LCR_SBS_1BIT | UART_LCR_PARITY_DIS));
	Chip_UART_SetupFIFOS(LPC_USART, (UART_FCR_FIFO_EN | UART_FCR_TRG_LEV2));
	Chip_UART_TXEnable(LPC_USART);

	if (SysTick_Config (SystemCoreClock / 1000)) {
		//Error
		while(1);
	}

	GPIO_Config();
	LED_Config();
	LED_On();

	while(1) {
		uint8_t bytes_read;
		uint8_t Rx_Buf[8];
		uint8_t init__buf;
		const Config_t config = {0,0,0,0};

		if ((bytes_read = Chip_UART_ReadBlocking(LPC_USART, init_buf, 1)) != 0) {
			if (init_buf == HELP_CHAR1 || init_buf == HELP_CHAR2) {
				print_help();
			} else if (init_buf == SET_CHAR) {
				
			} else if (init_buf == READ_CHAR) {
				print_config(&config);
			} else {
				const char *text = "Invalid character. Please try again";
				Chip_UART_SendBlocking(LPC_USART, text, strlen(text));
			}
		}
	}

	return 0;
}
