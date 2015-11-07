/**************************
* 	UART Echo Example     *
* 	Echoes out input      *
***************************/


#include "chip.h"
#include "stepperMotor.h"

const uint32_t OscRateIn = 0000000;

#define LED_PIN 10

volatile uint32_t msTicks;

uint8_t Rx_Buf[8];

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

	const uint8_t *string = "Hello World\n\r";

	if (SysTick_Config (SystemCoreClock / 1000)) {
		//Error
		while(1);
	}

	GPIO_Config();
	LED_Config();
	LED_On();
    Chip_GPIO_WriteDirBit(LPC_GPIO, 2, 10, true);
    
    uint8_t toggle = 0;

	STEPPER_MOTOR_T gauge;
	gauge.ports[0] = 0;
	gauge.ports[1] = 1;
	gauge.ports[2] = 2;
	gauge.ports[3] = 3;
	gauge.pins[0] = 2;
	gauge.pins[1] = 3;
	gauge.pins[2] = 7;
	gauge.pins[3] = 8;
	gauge.step_per_rotation = 640;
	gauge.step_delay = 10;
	Stepper_Init(&gauge);
	Stepper_ZeroPosition(&gauge, msTicks);

    while(1) {
		uint8_t count;
		if ((count = Chip_UART_Read(LPC_USART, Rx_Buf, 8)) != 0) {
			Chip_UART_SendBlocking(LPC_USART, Rx_Buf, count);
			if (Rx_Buf[0] == 'q') {
				Stepper_ChoosePosition(&gauge, 100, msTicks);
			}
			if (Rx_Buf[0] == 'z'){
				Stepper_Spin(&gauge,-640, msTicks);
			}
			if (Rx_Buf[0] == 'x'){
				Stepper_Spin(&gauge, 250, msTicks);
			}
			if (Rx_Buf[0] == 'a'){
				Stepper_HomePosition(&gauge, msTicks);
			}
		}
		Stepper_Step(&gauge, msTicks);
		    
            }
			
	return 0;
}
