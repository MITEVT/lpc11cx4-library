
#include "chip.h"
#include <stdio.h>

const uint32_t OscRateIn = 12000000;

#define LED_PIN 7
#define TICKRATE_HZ1 (10) /* 10 ticks per second */
#define TICKRATE_HZ2 (11) /* 11 ticks per second */

static void GPIO_Config(void) {
	Chip_GPIO_Init(LPC_GPIO);

}

static void LED_Config(void) {
	Chip_GPIO_WriteDirBit(LPC_GPIO, 0, LED_PIN, true);

}

static void LED_On(void) {
	Chip_GPIO_SetPinState(LPC_GPIO, 0, LED_PIN, true);
}

static void LED_Off(void) {
	Chip_GPIO_SetPinState(LPC_GPIO, 0, LED_PIN, false);
}


void SysTick_Handler(void)
{
	LED_Off();
}

void TIMER32_0_IRQHandler(void)
{
	if (Chip_TIMER_MatchPending(LPC_TIMER32_0, 1)) {
		Chip_TIMER_ClearMatch(LPC_TIMER32_0, 1);
		LED_On();
	}
}

/**
 * @brief	Main routine for SSP example
 * @return	Nothing
 */
int main(void)
{
	uint32_t timerFreq;

	SystemCoreClockUpdate();

	/* LED Initialization */
	GPIO_Config();
	LED_Config();

	SysTick_Config(SystemCoreClock / TICKRATE_HZ1);
	Chip_TIMER_Init(LPC_TIMER32_0);

	timerFreq = Chip_Clock_GetSystemClockRate();

	/* Timer setup for match and interrupt at TICKRATE_HZ */
	Chip_TIMER_Reset(LPC_TIMER32_0);
	Chip_TIMER_MatchEnableInt(LPC_TIMER32_0, 1);
	Chip_TIMER_SetMatch(LPC_TIMER32_0, 1, (timerFreq / TICKRATE_HZ2));
	Chip_TIMER_ResetOnMatchEnable(LPC_TIMER32_0, 1);
	Chip_TIMER_Enable(LPC_TIMER32_0);

	/* Enable timer interrupt */
	NVIC_ClearPendingIRQ(TIMER_32_0_IRQn);
	NVIC_EnableIRQ(TIMER_32_0_IRQn);


	
	while(1){
		__WFI();
	}
	
	return 0;
}
