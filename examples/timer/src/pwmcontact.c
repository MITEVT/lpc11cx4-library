
#include "chip.h"
const uint32_t OscRateIn = 12000000;
//30
#define LED_PIN 10

__INLINE static void GPIO_Config(void) {
	Chip_GPIO_Init(LPC_GPIO);

}
__INLINE static void LED_Config(void) {
	Chip_GPIO_WriteDirBit(LPC_GPIO, 2, LED_PIN, true);

}

__INLINE static void LED_On(void) {
	Chip_GPIO_SetPinState(LPC_GPIO, 2, LED_PIN, true);
}

volatile uint32_t msTicks;

void SysTick_Handler(void) {
	msTicks++;
}

__INLINE static void Delay(uint32_t dlyTicks) {
	uint32_t curTicks = msTicks;
	while ((msTicks - curTicks) < dlyTicks);
}

int main(void) {

	SystemCoreClockUpdate();


	if (SysTick_Config (SystemCoreClock / 1000)) {
		//Error
		while(1);
	}

	GPIO_Config();
 	LED_Config();
	LED_On();
	Chip_GPIO_SetPinDIRInput(LPC_GPIO, 0, 1);
	Chip_GPIO_SetPinDIROutput(LPC_GPIO, 1, 10);
	Chip_GPIO_SetPinState(LPC_GPIO, 1, 10, true);
	Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO1_10, (IOCON_FUNC0 | IOCON_MODE_INACT));/* TIMER1 PWM */
	Chip_TIMER_Init(LPC_TIMER16_1);
	Chip_TIMER_Reset(LPC_TIMER16_1);
	Chip_TIMER_PrescaleSet(LPC_TIMER16_1, 1);
	Chip_TIMER_SetMatch(LPC_TIMER16_1, 1, 96);
	Chip_TIMER_SetMatch(LPC_TIMER16_1, 2, 100);
	Chip_TIMER_SetMatch(LPC_TIMER16_1, 0, 100);
	Chip_TIMER_SetMatch(LPC_TIMER16_1, 4, 100);
	Chip_TIMER_ResetOnMatchEnable(LPC_TIMER16_1, 2);
	LPC_TIMER16_1->PWMC |= 0x02;
	Chip_TIMER_ExtMatchControlSet(LPC_TIMER16_1, 0, TIMER_EXTMATCH_TOGGLE, 1);
	//Chip_TIMER_Enable(LPC_TIMER16_1);

	bool chopping = false;

	while(1){
		if (!Chip_GPIO_GetPinState(LPC_GPIO, 0, 1)) {
			//Switch is at 0V
			if (chopping == true) {
				//Chopping already true
				//DO NOTHING
			}
			if (chopping == false) {
				//Chopping off
				//Set chopping high
				Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO1_10, (IOCON_FUNC0 | IOCON_MODE_INACT));
				Chip_GPIO_SetPinState(LPC_GPIO, 1, 10, false);
				//Delay
				Delay(1000);
				//Enable Timer
				Chip_TIMER_Enable(LPC_TIMER16_1);
				// Set bool to true
				Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO1_10, (IOCON_FUNC2 | IOCON_MODE_INACT));

				chopping = true;
			}
		}
		else {
			//Switch is at 3.3 V
			if (chopping == true){
				//Chopping is true
				//Stop Timer
				Chip_TIMER_Disable(LPC_TIMER16_1);
				Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO1_10, (IOCON_FUNC0 | IOCON_MODE_INACT));
				Chip_GPIO_SetPinState(LPC_GPIO, 1, 10, true);
				//Set Chopping to false
				chopping = false;
			}
			if (chopping == true){
				//Chopping is false
				//DO NOTHING
			}
		}



	}
}
//SWITCH at P1O_01
//setting contacter to high - GPIO 
//If switch is ON:
//Contactor chopping
//If switch is OFF:
//Turn off contactor chopping
