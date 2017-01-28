#include "chip.h"
#include "sysinit.h"

const uint32_t OscRateIn = 0;

#define LED0 2, 8
#define LED1 2, 7

volatile uint32_t msTicks;

void SysTick_Handler(void) {
	msTicks++;
}

static void Delay(uint32_t dlyTicks) {
	uint32_t curTicks = msTicks;
	while ((msTicks - curTicks) < dlyTicks);
}

int main (void) {

	SysTick_Config(TicksPerMS);

	Chip_GPIO_Init(LPC_GPIO);
	Chip_GPIO_WriteDirBit(LPC_GPIO, LED0, true);
	Chip_GPIO_WriteDirBit(LPC_GPIO, LED1, true);

	uint8_t i = 1;
	uint16_t delay = 500;

	while(1) {
		i = 1 - i;
		delay = (i) ? 500 : 250;

		Chip_GPIO_SetPinState(LPC_GPIO, LED1, false);
		Chip_GPIO_SetPinState(LPC_GPIO, LED0, true);
		Delay(delay);
		Chip_GPIO_SetPinState(LPC_GPIO, LED0, false);
		Chip_GPIO_SetPinState(LPC_GPIO, LED1, true);
		Delay(delay);
	}
}