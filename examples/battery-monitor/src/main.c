/**************************
* 	Reads Config Input    *
* 	for Battery System    *
***************************/

#include "chip.h"
#include "util.h"
#include <string.h>
#include <stdlib.h>

#define LED_PIN 7
#define MAX_VAL_LEN 10
#define HELP_CHAR 'h'
#define READ_CHAR 'r'
#define SET_CHAR 's'
#define SET_CHAR 's'
#define FAST_PRINT(str) Chip_UART_SendBlocking(LPC_USART, str, strlen(str))

struct Config {
	uint8_t series;
	uint8_t parallel;
	uint8_t modular_series;
	uint8_t modular_parallel;
};

typedef struct Config Config_t;

const uint32_t OscRateIn = 0000000;

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

void print_help(void) {
	const char *text = "Press 'r' to read settings. 's' to set the current battery settings, 'h' to repeat instructions.\n\r";
	FAST_PRINT(text);
}

void print_config(Config_t *config) {
	char container[MAX_VAL_LEN];

	FAST_PRINT("Series: ");
	itoa(config->series, container, MAX_VAL_LEN);
	FAST_PRINT(container);
	FAST_PRINT("\n\r");
	
	FAST_PRINT("Parallel: ");
	itoa(config->parallel, container, MAX_VAL_LEN);
	FAST_PRINT(container);
	FAST_PRINT("\n\r");

	FAST_PRINT("Modular Series: ");
	itoa(config->modular_series, container, MAX_VAL_LEN);
	FAST_PRINT(container);
	FAST_PRINT("\n\r");

	FAST_PRINT("Modular Parallel: ");
	itoa(config->modular_parallel, container, MAX_VAL_LEN);
	FAST_PRINT(container);
	FAST_PRINT("\n\r");
}

int reverse_int(int n) {
	int reverse = 0;
	while (n != 0) {
		reverse = reverse * 10;
		reverse = reverse + n%10;
		n = n/10;
	}
	return reverse;
}

uint8_t ask_for_input(char *question) {

	FAST_PRINT(question);
	FAST_PRINT("'s value? ");

	int curr_val = 0;
	uint8_t count = 0;
	uint8_t power_of_ten = 1;
	char read_buf[1];

	Chip_UART_ReadBlocking(LPC_USART, read_buf, 1);
	while (read_buf[0] != '\n' && read_buf[0] != '\r') {

		FAST_PRINT(read_buf);
		curr_val += ((int) read_buf[0] & 0xF)*power_of_ten;
		power_of_ten *= 10;

		count++;
		if (count==MAX_VAL_LEN-1) {
			break;
		}

		Chip_UART_ReadBlocking(LPC_USART, read_buf, 1);
	}

	FAST_PRINT("\n\r");

	int final = reverse_int(curr_val);

	return (uint8_t) final;
}

int main(void) {

	SystemCoreClockUpdate();

	Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO1_6, (IOCON_FUNC1 | IOCON_MODE_INACT));/* RXD */
	Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO1_7, (IOCON_FUNC1 | IOCON_MODE_INACT));/* TXD */

	Chip_UART_Init(LPC_USART);
	Chip_UART_SetBaud(LPC_USART, 9600);
	Chip_UART_ConfigData(LPC_USART, (UART_LCR_WLEN8 | UART_LCR_SBS_1BIT | UART_LCR_PARITY_DIS));
	Chip_UART_SetupFIFOS(LPC_USART, (UART_FCR_FIFO_EN | UART_FCR_TRG_LEV2));
	Chip_UART_TXEnable(LPC_USART);

	if (SysTick_Config (SystemCoreClock / 1000)) {
		while(1); //Error
	}

	GPIO_Config();
	Config_t config = {10,5,17,0};

	while(1) {
		char init_buf;

		Chip_UART_ReadBlocking(LPC_USART, &init_buf, 1);

		if (init_buf == HELP_CHAR) {
			print_help();

		} else if (init_buf == SET_CHAR) {

			config.series = ask_for_input("serial");
			config.parallel = ask_for_input("parallel");
			config.modular_series = ask_for_input("modular serial");
			config.modular_parallel = ask_for_input("modular parallel");

			print_help();

		} else if (init_buf == READ_CHAR) {
			print_config(&config);

		} else {
			const char *text = "Invalid character. 'h' for help. \n\r";
			FAST_PRINT(text);
		}
	}

	return 0;
}
