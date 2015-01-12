#include "LPC11xx.h"
// #include "system_LPC11xx.h"
// #include "system_LPC11xx.c"


#define LED_PIN 4

volatile uint32_t msTicks;

void SysTick_Handler(void) {
	msTicks++;
}

__INLINE static void Delay(uint32_t dlyTicks) {
	uint32_t curTicks = msTicks;
	while ((msTicks - curTicks) < dlyTicks);
}

__INLINE static void GPIO_Config(void) {
	LPC_SYSCON->SYSAHBCLKCTRL |= (1 << 6); //Enable GPIO Clock
}

__INLINE static void LED_Config(void) {
	LPC_GPIO0->DIR |= (1 << LED_PIN);
}

__INLINE static void LED_On(void) {
	LPC_GPIO0->DATA |= (1 << LED_PIN);
}

__INLINE static void LED_Off(void) {
	LPC_GPIO0->DATA &= ~(1 << LED_PIN);
}

int main (void) {
	if (SysTick_Config (SystemCoreClock / 1000)) {
		//Error
		while(1);
	}

	GPIO_Config();
	LED_Config();

	while(1) {
		LED_On();
		Delay(100);
		LED_Off();
		Delay(100);
	}
}