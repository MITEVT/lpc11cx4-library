#include "chip.h"
#include "util.h"
#include <string.h>
#include "can.h"
#include <stdlib.h>

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/

#define LED_PORT 0
#define LED_PIN 7

#define UART_RX_BUFFER_SIZE 8

const uint32_t OscRateIn = 12000000;


volatile uint32_t msTicks;

CCAN_MSG_OBJ_T rx_msg;
static char str[100];
static char uart_rx_buf[UART_RX_BUFFER_SIZE];

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

static void Print_Buffer(uint8_t* buff, uint8_t buff_size) {
    Chip_UART_SendBlocking(LPC_USART, "0x", 2);
    uint8_t i;
    for(i = 0; i < buff_size; i++) {
        itoa(buff[i], str, 16);
        if(buff[i] < 16) {
            Chip_UART_SendBlocking(LPC_USART, "0", 1);
        }
        Chip_UART_SendBlocking(LPC_USART, str, 2);
    }
}


int main(void) {
	SystemCoreClockUpdate();

	if (SysTick_Config (SystemCoreClock / 1000)) {
		//Error
		while(1);
	}


	Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO1_6, (IOCON_FUNC1 | IOCON_MODE_INACT)); /* RXD */
	Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO1_7, (IOCON_FUNC1 | IOCON_MODE_INACT)); /* TXD */

	Chip_UART_Init(LPC_USART);
	Chip_UART_SetBaud(LPC_USART, 57600);
	Chip_UART_ConfigData(LPC_USART, (UART_LCR_WLEN8 | UART_LCR_SBS_1BIT | UART_LCR_PARITY_DIS));
	Chip_UART_SetupFIFOS(LPC_USART, (UART_FCR_FIFO_EN | UART_FCR_TRG_LEV2));
	Chip_UART_TXEnable(LPC_USART);

	DEBUG_Print("Started up\n\r");

	CAN_Init(500000);

	uint32_t ret;

	while (1) {
		uint8_t count;
		uint8_t data[1];

		if (msTicks % 1000 == 0){
            // recieve message if there is a message
		    ret = CAN_Receive(&rx_msg);
            if(ret == NO_RX_CAN_MESSAGE) {
                DEBUG_Print("No CAN message received...\r\n");
            } else if(ret == NO_CAN_ERROR) {
                DEBUG_Print("Recieved data ");
                Print_Buffer(rx_msg.data, rx_msg.dlc);
                DEBUG_Print(" from ");
                itoa(rx_msg.mode_id, str, 16);
                DEBUG_Print(str);
                DEBUG_Print("\r\n");
            } else {
                DEBUG_Print("CAN Error: ");
                itoa(ret, str, 2);
                DEBUG_Print(str);
                DEBUG_Print("\r\n");
            }

            // transmit a message!
		    data[0] = 0xAA;
		    CAN_Transmit(0x600, data, 1);
		}
        
		if ((count = Chip_UART_Read(LPC_USART, uart_rx_buf, UART_RX_BUFFER_SIZE)) != 0) {
			switch (uart_rx_buf[0]) {
				case 'a':
					DEBUG_Print("Sending CAN with ID: 0x600\r\n");
					data[0] = 0xAA;
					ret = CAN_Transmit(0x600, data, 1);
                    if(ret != NO_CAN_ERROR) {
                        DEBUG_Print("CAN Error: ");
					    itoa(ret, str, 2);
					    DEBUG_Print(str);
                        DEBUG_Print("\r\n");
                    }
					break;
				default:
					DEBUG_Print("Invalid Command\r\n");
					break;
			}
		}
	}
}
