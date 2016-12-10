#include "chip.h"
#include <string.h>

const uint32_t OscRateIn = 0;

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/

#define UART_BAUD_RATE 115200

#define LED0 2, 8
#define LED1 2, 7

volatile uint32_t msTicks;

#define UART_BUFFER_SIZE 9
static RINGBUFF_T ring;
static volatile uint8_t _ring[UART_BUFFER_SIZE];

static uint8_t uart_buf[UART_BUFFER_SIZE];

/*****************************************************************************
 * Private functions
 ****************************************************************************/

void SysTick_Handler(void) {
	msTicks++;
}


int main(void)
{

	SystemCoreClockUpdate();
	SysTick_Config(SystemCoreClock / 1000);

	// Initialize UART Buffers
	RingBuffer_Init(&ring, _ring, sizeof(uint8_t), UART_BUFFER_SIZE);
	RingBuffer_Flush(&ring);

	Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO1_6, (IOCON_FUNC1 | IOCON_MODE_INACT));/* RXD */
	Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO1_7, (IOCON_FUNC1 | IOCON_MODE_INACT));/* TXD */

	Chip_UART_Init(LPC_USART);
	Chip_UART_SetBaud(LPC_USART, UART_BAUD);
	Chip_UART_ConfigData(LPC_USART, (UART_LCR_WLEN8 | UART_LCR_SBS_1BIT | UART_LCR_PARITY_DIS));
	Chip_UART_SetupFIFOS(LPC_USART, (UART_FCR_FIFO_EN | UART_FCR_TRG_LEV1));
	Chip_UART_TXEnable(LPC_USART);


	Chip_GPIO_Init(LPC_GPIO);
	Chip_GPIO_WriteDirBit(LPC_GPIO, LED0, true);
	Chip_GPIO_WriteDirBit(LPC_GPIO, LED1, true);

	uint32_t last_flip = 0;

	uint8_t long_message[40];
	uint8_t short_message[10] = "Started Up";

	Chip_UART_SendBlocking(LPC_USART, "Hello World\r\n", 13);

	uint8_t i;
	for (i = 0; i < 10; i++) {

		uint8_t tmp = RingBuffer_Insert(&ring, short_message + i);
		Chip_UART_SendBlocking(LPC_USART, short_message+i, 1);
		if (tmp) {
			Chip_UART_SendBlocking(LPC_USART, " 1 ", 3);
		} else {
			Chip_UART_SendBlocking(LPC_USART, " 0 ", 3);
		}
		itoa(ring.head, long_message, 10);
		Chip_UART_SendBlocking(LPC_USART, long_message, 2);
		Chip_UART_SendBlocking(LPC_USART, " ", 1);
		itoa(ring.head % (ring.count), long_message, 10);
		Chip_UART_SendBlocking(LPC_USART, long_message, 2);
		Chip_UART_SendBlocking(LPC_USART, ",", 1);

	}

	Chip_UART_SendBlocking(LPC_USART, "\r\n", 2);

	uint8_t buffered_message[10];
	for (i = 0; i < 10; i++) {
		uint8_t ch;
		RingBuffer_Pop(&ring, &ch);
		buffered_message[i] = ch;
		Chip_UART_SendBlocking(LPC_USART, &ch, 1);
		Chip_UART_SendBlocking(LPC_USART, ",", 1);
	}

	Chip_UART_SendBlocking(LPC_USART, "\r\n", 2);

	Chip_UART_SendBlocking(LPC_USART, buffered_message, 10);

	last_flip = msTicks;
	while(msTicks - last_flip < 1000) {

	}

	last_flip = msTicks;
	while(msTicks - last_flip < 1000) {

	}

	while (1) {
	}
}
