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

#define UART_BUFFER_SIZE 42
static RINGBUFF_T uart_rx_ring;
static volatile uint8_t _uart_rx_ring[UART_BUFFER_SIZE];
static RINGBUFF_T uart_tx_ring;
static volatile uint8_t _uart_tx_ring[UART_BUFFER_SIZE];

static uint8_t uart_buf[UART_BUFFER_SIZE];

/*****************************************************************************
 * Private functions
 ****************************************************************************/

void SysTick_Handler(void) {
	msTicks++;
}

void UART_IRQHandler(void) {
	Chip_UART_IRQRBHandler(LPC_USART, &uart_rx_ring, &uart_tx_ring);
}


int main(void)
{

	SystemCoreClockUpdate();
	SysTick_Config(SystemCoreClock / 1000);

	// Initialize UART Buffers
	RingBuffer_Init(&uart_rx_ring, _uart_rx_ring, sizeof(uint8_t), UART_BUFFER_SIZE);
	RingBuffer_Flush(&uart_rx_ring);
	RingBuffer_Init(&uart_tx_ring, _uart_tx_ring, sizeof(uint8_t), UART_BUFFER_SIZE);
	RingBuffer_Flush(&uart_tx_ring);

	Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO1_6, (IOCON_FUNC1 | IOCON_MODE_INACT));/* RXD */
	Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO1_7, (IOCON_FUNC1 | IOCON_MODE_INACT));/* TXD */

	Chip_UART_Init(LPC_USART);
	Chip_UART_SetBaud(LPC_USART, UART_BAUD);
	Chip_UART_ConfigData(LPC_USART, (UART_LCR_WLEN8 | UART_LCR_SBS_1BIT | UART_LCR_PARITY_DIS));
	Chip_UART_SetupFIFOS(LPC_USART, (UART_FCR_FIFO_EN | UART_FCR_TRG_LEV1));
	Chip_UART_TXEnable(LPC_USART);

	Chip_UART_IntEnable(LPC_USART, UART_IER_RBRINT);
	NVIC_ClearPendingIRQ(UART0_IRQn);
	NVIC_EnableIRQ(UART0_IRQn);

	Chip_GPIO_Init(LPC_GPIO);
	Chip_GPIO_WriteDirBit(LPC_GPIO, LED0, true);
	Chip_GPIO_WriteDirBit(LPC_GPIO, LED1, true);

	uint32_t last_flip = 0;

	uint8_t long_message[14] = "abcdef123456\r\n";

	uint32_t n = Chip_UART_SendRB(LPC_USART, &uart_tx_ring, "Started Up\r\n", 12);
	last_flip = msTicks;
	while(msTicks - last_flip < 1000) {

	}

	while (1) {
		uint8_t count;
		if ((count = Chip_UART_ReadRB(LPC_USART, &uart_rx_ring, uart_buf, UART_BUFFER_SIZE)) != 0) {
			Chip_UART_SendRB(LPC_USART, &uart_tx_ring, uart_buf, count);
		}

		if (msTicks - last_flip > 0) {
			last_flip = msTicks;
			Chip_GPIO_SetPinState(LPC_GPIO, LED0, 1-Chip_GPIO_GetPinState(LPC_GPIO, LED0));
			Chip_UART_SendRB(LPC_USART, &uart_tx_ring, long_message, 14);
		}


	}
}
