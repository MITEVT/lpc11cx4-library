/**************************
* 	Reads Config Input    *
* 	for Battery System    *
***************************/

struct Config {
	uint8_t series;
	uint8_t parallel;
	uint8_t modular_series;
	uint8_t modular_parallel;
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
	const uint8_t *text = "Press 'r' to read the current battery settings.\n"
						"Press 's' to set the current battery settings.\n"
						"Press '?' or 'h' to repeat these instructions.\n\r";
	Chip_UART_SendBlocking(LPC_USART, text, strlen(text));
}

void print_config(Config_t *config) {
	//This is really ugly. Better way to do it?
	uint8_t text[80];
	strcpy(text, "Series: ");
	strcat(text, to_string(&config.series));
	strcpy(text, "\nParallel: ");
	strcat(text, to_string(&config.parallel));
	strcpy(text, "\nModular Series: ");
	strcat(text, to_string(&config.modular_series));
	strcpy(text, "\nModular Parallel: ");
	strcat(text, to_string(&config.modular_parallel));
	strcpy(text, "\n");
	Chip_UART_SendBlocking(LPC_USART, &text, strlen(text));
}

uint8_t ask_for_input(uint8_t *question) {

	//Ask for value
	Chip_UART_SendBlocking(LPC_USART, question, strlen(question));
	uint8_t *ending = "'s value? "
	Chip_UART_SendBlocking(LPC_USART, ending, strlen(ending));
	*ending = "\n"
	Chip_UART_SendBlocking(LPC_USART, ending, strlen(ending));

	uint8_t str_val[10] = "0";
	uint8_t count = 0;
	uint8_t read_buf;
	Chip_UART_ReadBlocking(LPC_USART, read_buf, 1);
	while (read_buf != '\n' || read_buf != '\r') {
		str_val[count] = read_buf;
		count++;
		if (count==10) {
			break;
		}
	}
	return atoi(str_val);
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

		Chip_UART_ReadBlocking(LPC_USART, init_buf, 1)

		if (init_buf == HELP_CHAR1 || init_buf == HELP_CHAR2) {
			print_help();

		} else if (init_buf == SET_CHAR) {
			config.series = ask_for_input("serial");
			config.parallel = ask_for_input("parallel");
			config.modular_serial = ask_for_input("modular serial");
			config.modular_parallel = ask_for_input("modular parallel");

		} else if (init_buf == READ_CHAR) {
			print_config(&config);

		} else {
			const uint8_t *text = "Invalid character. Please try again";
			Chip_UART_SendBlocking(LPC_USART, text, strlen(text));
		}
	}

	return 0;
}
