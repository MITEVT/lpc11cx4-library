/**************************
* 	Reads Config Input    *
* 	for Battery System    *
***************************/


#include "chip.h"
#include <string.h>
#include <stdlib.h>

#define LED_PIN 7
#define HELP_CHAR1 '?'
#define HELP_CHAR2 'h'
#define READ_CHAR 'r'
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

char* itoa(int num, char* str, int base)
{
    int i = 0;
    bool isNegative = false;
 
    /* Handle 0 explicitely, otherwise empty string is printed for 0 */
    if (num == 0)
    {
        str[i++] = '0';
        str[i] = '\0';
        return str;
    }
 
    // In standard itoa(), negative numbers are handled only with 
    // base 10. Otherwise numbers are considered unsigned.
    if (num < 0 && base == 10)
    {
        isNegative = true;
        num = -num;
    }
 
    // Process individual digits
    while (num != 0)
    {
        int rem = num % base;
        str[i++] = (rem > 9)? (rem-10) + 'a' : rem + '0';
        num = num/base;
    }
 
    // If number is negative, append '-'
    if (isNegative)
        str[i++] = '-';
 
    str[i] = '\0'; // Append string terminator
 
    // Reverse the string
    reverse(str, i);
 
    return str;
}

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
	const char *text = "Press 'r' to read the current battery settings. "
						"Press 's' to set the current battery settings. "
						"Press '?' or 'h' to repeat these instructions.\n\r";
	Chip_UART_SendBlocking(LPC_USART, text, strlen(text));
}

void print_config(Config_t *config) {
	//This is really ugly. Better way to do it?
	char *container;

	FAST_PRINT("Series: ");
	//itoa(config->series, container, 10);
	int i = 5;
	itoa(i, container, 10);
	FAST_PRINT(container);
	
	FAST_PRINT("Parallel: ");
	itoa(config->parallel, container, 10);
	FAST_PRINT(container);

	FAST_PRINT("Modular Series: ");
	itoa(config->modular_series, container, 10);
	FAST_PRINT(container);

	FAST_PRINT("Modular Parallel: ");
	itoa(config->modular_parallel, container, 10);
	FAST_PRINT(container);
}

uint8_t ask_for_input(char *question) {

	//Ask for value
	FAST_PRINT(question);
	FAST_PRINT("'s value? ");
	FAST_PRINT("\n");

	char str_val[10] = "0";
	uint8_t count = 0;
	char read_buf;
	Chip_UART_ReadBlocking(LPC_USART, &read_buf, 1);
	while (read_buf != '\n' || read_buf != '\r') {
		str_val[count] = read_buf;
		count++;
		if (count==10) {
			break;
		}
	}
	return (uint8_t) atoi(str_val);
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
		char init_buf;
		Config_t config = {0,0,0,0};

		Chip_UART_ReadBlocking(LPC_USART, &init_buf, 1);

		if (init_buf == HELP_CHAR1 || init_buf == HELP_CHAR2) {
			print_help();

		} else if (init_buf == SET_CHAR) {
			config.series = ask_for_input("serial");
			config.parallel = ask_for_input("parallel");
			config.modular_series = ask_for_input("modular serial");
			config.modular_parallel = ask_for_input("modular parallel");

		} else if (init_buf == READ_CHAR) {
			print_config(&config);

		} else {
			const char *text = "Invalid character. Please try again. 'h' for help. \n\r";
			Chip_UART_SendBlocking(LPC_USART, text, strlen(text));
		}
	}

	return 0;
}
