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

    uint32_t reset_can_peripheral_time;
    const uint32_t can_error_delay = 500;
    bool reset_can_peripheral = false;

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
    Chip_GPIO_SetPinDIR(LPC_GPIO, 1, 8, 1);
    Chip_GPIO_SetPinDIR(LPC_GPIO, 2, 7, 1);
    Chip_GPIO_SetPinDIR(LPC_GPIO, 2, 8, 1);

	CAN_Init(500000);

	uint32_t ret;
    uint32_t last_send = msTicks;
    uint16_t id = 0;
	while (1) {
		uint8_t count;
		uint8_t data[4];

        if(reset_can_peripheral && msTicks > reset_can_peripheral_time) {
            DEBUG_Print("Attempting to reset CAN peripheral...\r\n");
            CAN_ResetPeripheral();
            CAN_Init(500000);
            DEBUG_Print("Reset CAN peripheral. \r\n");
            reset_can_peripheral = false;
        }

        // recieve message if there is a message
        ret = CAN_Receive(&rx_msg);
        if(ret == NO_CAN_ERROR) {
            Chip_GPIO_SetPinState(LPC_GPIO, 2, 7, 1);
            DEBUG_Print("R ");
            Print_Buffer(rx_msg.data, rx_msg.dlc);
            DEBUG_Print(" id ");
            itoa(rx_msg.mode_id, str, 16);
            DEBUG_Print(str);
            // // DEBUG_Print(" with datalen ");
            // // itoa(rx_msg.dlc, str, 16);
            // // DEBUG_Print(str);
            // itoa((LPC_CCAN->CANIF1_MCTRL >> 14) & 1, str, 2);
            // DEBUG_Print(str);
            // DEBUG_Print(" ");
            // itoa((LPC_CCAN->CANIF2_MCTRL >> 14) & 1, str, 2);
            // DEBUG_Print(str);
            DEBUG_Print("\r\n");
            Chip_GPIO_SetPinState(LPC_GPIO, 2, 7, 0);
        } else if (ret == NO_RX_CAN_MESSAGE) {

        } else {
            DEBUG_Print("CAN Error (Rx): ");
            itoa(ret, str, 2);
            DEBUG_Print(str);
            DEBUG_Print("\r\n");

            DEBUG_Print("Will attempt to reset peripheral in ");
            itoa(can_error_delay/1000, str, 10);
            DEBUG_Print(str);
            DEBUG_Print(" seconds.\r\n");
            reset_can_peripheral = true;
            reset_can_peripheral_time = msTicks + can_error_delay;
        }

        
        if (msTicks - last_send > 20) {
            DEBUG_Print("---------\r\n");
            last_send = msTicks;
            // Chip_GPIO_SetPinState(LPC_GPIO, 2, 8, 1);
            // int i;
            // for (i = 0; i < 3; i++) {
            //     data[0] = 0x11+i;
            //     data[1] = 0x33+i;
            //     data[2] = 0x55+i;
            //     data[3] = 0x77+i;
            //     ret = CAN_Transmit(id % 0x600, data, 4);
            //     id++;
            //     if(ret != NO_CAN_ERROR) {
            //         DEBUG_Print("CAN Error (Tx): ");
            //         itoa(ret, str, 2);
            //         DEBUG_Print(str);
            //         DEBUG_Print("\r\n");
            //     }
            // }
        }
        Chip_GPIO_SetPinState(LPC_GPIO, 2, 8, 0);
        
		if ((count = Chip_UART_Read(LPC_USART, uart_rx_buf, UART_RX_BUFFER_SIZE)) != 0) {
			switch (uart_rx_buf[0]) {
				case 'a':
					DEBUG_Print("Sending CAN with ID: 0x600\r\n");
					data[0] = 0xAA;
					ret = CAN_Transmit(0x600, data, 1);
                    if(ret != NO_CAN_ERROR) {
                        DEBUG_Print("CAN Error (Tx): ");
					    itoa(ret, str, 2);
					    DEBUG_Print(str);
                        DEBUG_Print("\r\n");
                    }
					break;
                case 'b':
                    DEBUG_Print("Sending CAN with ID 0x601 and 0x602\r\n");
                    // uint8_t busy_one, busy_two;
                    data[0] = 0xAA;
                    ret = CAN_Transmit(0x601, data, 1);
                    if(ret != NO_CAN_ERROR) {
                        DEBUG_Print("CAN Error (Tx): ");
                        itoa(ret, str, 2);
                        DEBUG_Print(str);
                        DEBUG_Print("\r\n");
                    }
                    data[0] = 0x33;
                    ret = CAN_Transmit(0x602, data, 1);
                    if(ret != NO_CAN_ERROR) {
                        DEBUG_Print("CAN Error (Tx): ");
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
