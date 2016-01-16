
#include "chip.h"

const uint32_t OscRateIn = 12000000;

#define LED0 2, 5
#define LED1 0, 6
#define LED2 0, 7
#define LED3 2, 9

volatile uint32_t msTicks;

void SysTick_Handler(void) {
	msTicks++;
}

static void Delay(uint32_t dlyTicks) {
	uint32_t curTicks = msTicks;
	while ((msTicks - curTicks) < dlyTicks);
}

static void GPIO_Config(void) {
	Chip_GPIO_Init(LPC_GPIO);

}

static void LED_Init(uint8_t port, uint8_t pin) {
	Chip_GPIO_WriteDirBit(LPC_GPIO, port, pin, true);
	Chip_GPIO_SetPinState(LPC_GPIO, port, pin, false);

}

static void LED_Write(uint8_t port, uint8_t pin, uint8_t val) {
	Chip_GPIO_SetPinState(LPC_GPIO, port, pin, val);
}

int main (void) {

	SystemCoreClockUpdate();


	if (SysTick_Config (SystemCoreClock / 1000)) {
		//Error
		while(1);
	}

	

	LED_Init(LED0);
	LED_Init(LED1);
	LED_Init(LED2);
	LED_Init(LED3);

	GPIO_Config();

	while(1) {
		LED_Write(LED3, false);
		LED_Write(LED0, true);
		Delay(1000);
		LED_Write(LED0, false);
		LED_Write(LED1, true);
		Delay(1000);
		LED_Write(LED1, false);
		LED_Write(LED2, true);
		Delay(1000);
		LED_Write(LED2, false);
		LED_Write(LED3, true);
		Delay(1000);
	}
}