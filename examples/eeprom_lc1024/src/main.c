#include "chip.h"
#include "util.h"
#include "lc1024.h"
#include <string.h>

const uint32_t OscRateIn = 0;

#define SPI_BUFFER_SIZE 12
#define SSP_IRQ           SSP1_IRQn
#define SSPIRQHANDLER     SSP1_IRQHandler
#define ADDR_LEN 3
#define MAX_DATA_LEN 16

#define LED0 2, 11
volatile uint32_t msTicks;

static char str[100];

static uint8_t Rx_Buf[SPI_BUFFER_SIZE];
static uint8_t data[MAX_DATA_LEN];
static uint8_t address[ADDR_LEN];

static void PrintRxBuffer(void);
static void ZeroRxBuf(void);

void SysTick_Handler(void) {
	msTicks++;
}

static void delay(uint32_t dlyTicks) {
	uint32_t curTicks = msTicks;
	while ((msTicks - curTicks) < dlyTicks);
}

static void GPIO_Config(void) {
	Chip_GPIO_Init(LPC_GPIO);
}

static void LED_Config(void) {
	Chip_GPIO_WriteDirBit(LPC_GPIO, LED0, true);
}

static void PrintRxBuffer(void) {
    Chip_UART_SendBlocking(LPC_USART, "0x", 2);
    uint8_t i;
    for(i = 0; i < SPI_BUFFER_SIZE; i++) {
        itoa(Rx_Buf[i], str, 16);
        if(Rx_Buf[i] < 16) {
            Chip_UART_SendBlocking(LPC_USART, "0", 1);
        }
        Chip_UART_SendBlocking(LPC_USART, str, 2);
    }
    Chip_UART_SendBlocking(LPC_USART, "\n", 1);
}

static void uart_init(void) {
	Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO1_6, (IOCON_FUNC1 | IOCON_MODE_INACT));/* RXD */
	Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO1_7, (IOCON_FUNC1 | IOCON_MODE_INACT));/* TXD */

	Chip_UART_Init(LPC_USART);
	Chip_UART_SetBaud(LPC_USART, 57600);
	Chip_UART_ConfigData(LPC_USART, (UART_LCR_WLEN8 | UART_LCR_SBS_1BIT | UART_LCR_PARITY_DIS));
	Chip_UART_SetupFIFOS(LPC_USART, (UART_FCR_FIFO_EN | UART_FCR_TRG_LEV2));
	Chip_UART_TXEnable(LPC_USART);
}

static void ZeroRxBuf(void) {
	uint8_t i;
	for (i = 0; i < SPI_BUFFER_SIZE; i++) {
		Rx_Buf[i] = 0;
	}
}

int main(void) {
	SystemCoreClockUpdate();

	if (SysTick_Config (SystemCoreClock / 1000)) {
		while(1); // error state
	}

    address[0] = 0x00;
    address[1] = 0x00;
    address[2] = 0x00;

	GPIO_Config();
	LED_Config();

    uart_init();
    LC1024_Init(LPC_SSP1, 600000, 2, 9);
    
    ZeroRxBuf();

    // LC1024_WriteDisable();
    // LC1024_ReadStatusReg(&Rx_Buf[0]);
    // PrintRxBuffer();

    LC1024_WriteEnable();
    LC1024_ReadStatusReg(&Rx_Buf[0]);
    Chip_UART_SendBlocking(LPC_USART, "STATUS REG:\n", 12);
    PrintRxBuffer();

    data[0] = 0x0E;
    data[1] = 0x0E;
    data[2] = 0x0E;
    data[3] = 0x0E;
    data[4] = 0x0E;
    data[5] = 0x0E;
    data[6] = 0x0E;
    data[7] = 0x0E;
    data[8] = 0x0E;
    data[9] = 0x0E;
    data[10] = 0x0E;

    LC1024_WriteMem(&address, &data, 10);
    Chip_UART_SendBlocking(LPC_USART, "writing D to memory...\n", 23);
    // PrintRxBuffer();

    delay(5);

    LC1024_ReadMem(&address, &Rx_Buf, 10);
    
    if(Rx_Buf[0] == 0xE) {
        Chip_UART_SendBlocking(LPC_USART, "Passed write test 1!\n", 21);
    } else {
        Chip_UART_SendBlocking(LPC_USART, "Failed write test 1 :(\n", 23);
        PrintRxBuffer();
    }

    data[0] = 0x0A;
    data[1] = 0x0A;

    LC1024_WriteMem(&address, &data, 2);
    Chip_UART_SendBlocking(LPC_USART, "writing A to memory...\n", 23);

    delay(5);

    LC1024_ReadMem(&address, &Rx_Buf, 2);

    if(Rx_Buf[1] == 0xA) {
        Chip_UART_SendBlocking(LPC_USART, "Passed write test 2!\n", 21);
    } else {
        Chip_UART_SendBlocking(LPC_USART, "Failed write test 2 :(\n", 23);
        PrintRxBuffer();
    }

	while(1) {
		delay(5);
	}

	return 0;
}

