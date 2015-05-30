#include "chip.h"

const uint32_t OscRateIn = 12000000;

#define LED 2, 10

volatile uint32_t msTicks;

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
	// Chip_GPIO_WriteDirBit(LPC_GPIO, 2, LED_PIN, true);
	Chip_GPIO_WriteDirBit(LPC_GPIO, LED, true);

}

__INLINE static void LED_On(void) {
	Chip_GPIO_SetPinState(LPC_GPIO, LED, true);
}

__INLINE static void LED_Off(void) {
	Chip_GPIO_SetPinState(LPC_GPIO, LED, false);
}

static ADC_CLOCK_SETUP_T adc_setup;

int main (void) {

	SystemCoreClockUpdate();


	if (SysTick_Config (SystemCoreClock / 1000)) {
		//Error
		while(1);
	}

	GPIO_Config();
	LED_Config();

	Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO0_11, FUNC2);

	Chip_ADC_Init(LPC_ADC, &adc_setup);
	Chip_ADC_EnableChannel(LPC_ADC, ADC_CH0, ENABLE);

	uint16_t adc_data = 0;
	while(1) {
		/* Start A/D conversion */
		Chip_ADC_SetStartMode(LPC_ADC, ADC_START_NOW, ADC_TRIGGERMODE_RISING);

		/* Waiting for A/D conversion complete */
		while (!Chip_ADC_ReadStatus(LPC_ADC, ADC_CH0, ADC_DR_DONE_STAT)) {}
		/* Read ADC value */
		Chip_ADC_ReadValue(LPC_ADC, ADC_CH0, &adc_data);

		if (adc_data > 0x0200) {
			LED_On();
		} else {
			LED_Off();
		}
	}
}








