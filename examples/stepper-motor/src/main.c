/**************************
* 	UART Echo Example     *
* 	Echoes out input      *
***************************/


#include "chip.h"
#include "stepperMotor.h"
#include "util.h"
#include <string.h>

const uint32_t OscRateIn = 0000000;

#define LED 2, 10

volatile uint32_t msTicks;

uint8_t Rx_Buf[8];
char str[32];

void SysTick_Handler(void) {
	msTicks++;
}

int main(void)
{

	SystemCoreClockUpdate();

	Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO1_6, (IOCON_FUNC1 | IOCON_MODE_INACT));/* RXD */
	Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO1_7, (IOCON_FUNC1 | IOCON_MODE_INACT));/* TXD */

	Chip_UART_Init(LPC_USART);
	Chip_UART_SetBaud(LPC_USART, 57600);
	Chip_UART_ConfigData(LPC_USART, (UART_LCR_WLEN8 | UART_LCR_SBS_1BIT | UART_LCR_PARITY_DIS));
	Chip_UART_SetupFIFOS(LPC_USART, (UART_FCR_FIFO_EN | UART_FCR_TRG_LEV2));
	Chip_UART_TXEnable(LPC_USART);

	Chip_UART_SendBlocking(LPC_USART, "Started Up\r\n", 12);

	if (SysTick_Config (SystemCoreClock / 1000)) {
		//Error
		while(1);
	}

	Chip_GPIO_Init(LPC_GPIO);
    Chip_GPIO_WriteDirBit(LPC_GPIO, LED, true);
    Chip_GPIO_SetPinState(LPC_GPIO, LED, true);

	STEPPER_MOTOR_T gauge1;
	STEPPER_MOTOR_T gauge2;

	gauge1.ports[0] = 3;
	gauge1.ports[1] = 2;
	gauge1.ports[2] = 2;
	gauge1.ports[3] = 0;
	gauge1.pins[0] = 0;
	gauge1.pins[1] = 2;
	gauge1.pins[2] = 7;
	gauge1.pins[3] = 2;
	gauge1.step_per_rotation = 640;
	gauge1.step_delay = 2;
	Stepper_Init(&gauge1);
	Stepper_ZeroPosition(&gauge1, msTicks);

	gauge2.ports[0] = 2;
	gauge2.ports[1] = 3;
	gauge2.ports[2] = 1;
	gauge2.ports[3] = 3;
	gauge2.pins[0] = 11;
	gauge2.pins[1] = 2;
	gauge2.pins[2] = 10;
	gauge2.pins[3] = 0;
	gauge2.step_per_rotation = 640;
	gauge2.step_delay = 2;
	Stepper_Init(&gauge2);
	Stepper_ZeroPosition(&gauge2, msTicks);

    while(1) {
		uint8_t count;
		if ((count = Chip_UART_Read(LPC_USART, Rx_Buf, 8)) != 0) {
			Chip_UART_SendBlocking(LPC_USART, Rx_Buf, count);
			if (Rx_Buf[0] == 'q') {
				Chip_UART_SendBlocking(LPC_USART, "50%\r\n", 5);
				Stepper_SetPosition(&gauge1, 50, msTicks);
			}
			if (Rx_Buf[0] == '0') {
				Chip_UART_SendBlocking(LPC_USART, "0%\r\n", 4);
				Stepper_SetPosition(&gauge1, 0, msTicks);
			}
			if (Rx_Buf[0] == '1') {
				Chip_UART_SendBlocking(LPC_USART, "100%\r\n", 6);
				Stepper_SetPosition(&gauge1, 100, msTicks);
			}
			if (Rx_Buf[0] == 'z'){
				Chip_UART_SendBlocking(LPC_USART, "-100\r\n", 6);
				Stepper_Spin(&gauge1, -100, msTicks);
			}
			if (Rx_Buf[0] == 'x'){
				Chip_UART_SendBlocking(LPC_USART, "+100\r\n", 6);
				Stepper_Spin(&gauge1, 100, msTicks);
			}
			if (Rx_Buf[0] == 'a'){
				Chip_UART_SendBlocking(LPC_USART, "Home\r\n", 6);
				Stepper_HomePosition(&gauge1, msTicks);
			}
			if (Rx_Buf[0] == 'p') {
				itoa(gauge1.pos, str, 10);
				Chip_UART_SendBlocking(LPC_USART, "Pos: ", 5);
				Chip_UART_SendBlocking(LPC_USART, str, strlen(str));
				Chip_UART_SendBlocking(LPC_USART, "\r\n", 2);

			}
			if (Rx_Buf[0] == 'w') {
				Chip_UART_SendBlocking(LPC_USART, "50%\r\n", 5);
				Stepper_SetPosition(&gauge2, 50, msTicks);
			}
			if (Rx_Buf[0] == 'e') {
				Chip_UART_SendBlocking(LPC_USART, "0%\r\n", 4);
				Stepper_SetPosition(&gauge2, 0, msTicks);
			}
			if (Rx_Buf[0] == 's') {
				Chip_UART_SendBlocking(LPC_USART, "100%\r\n", 6);
				Stepper_SetPosition(&gauge2, 100, msTicks);
			}
			if (Rx_Buf[0] == 'd'){
				Chip_UART_SendBlocking(LPC_USART, "-100\r\n", 6);
				Stepper_Spin(&gauge2, -100, msTicks);
			}
			if (Rx_Buf[0] == 'c'){
				Chip_UART_SendBlocking(LPC_USART, "+100\r\n", 6);
				Stepper_Spin(&gauge2, 100, msTicks);
			}
			if (Rx_Buf[0] == 'f'){
				Chip_UART_SendBlocking(LPC_USART, "Home\r\n", 6);
				Stepper_HomePosition(&gauge2, msTicks);
			}
			if (Rx_Buf[0] == 'i') {
				itoa(gauge2.pos, str, 10);
				Chip_UART_SendBlocking(LPC_USART, "Pos: ", 5);
				Chip_UART_SendBlocking(LPC_USART, str, strlen(str));
				Chip_UART_SendBlocking(LPC_USART, "\r\n", 2);
			}
		}

		Stepper_Step(&gauge1, msTicks);
		Stepper_Step(&gauge2, msTicks);
		    
	}
			
	return 0;
}
