#include "chip.h"
#include "util.h"
#include <string.h>


#define TEST_PORT 2
#define TEST_PIN 10

/*
Times the interrupts of on Pin 9
*/

const uint32_t OscRateIn = 12000000;
volatile uint32_t msTicks;

static char str[20];		// The array of chars for UART output

volatile static uint32_t time;	// The current running average fo time
volatile static uint32_t count;	// The number of samples taken since the last output
static uint32_t lastPrint;	// The last time for the output

void SysTick_Handler(void){
	msTicks++;
}

/*
* During the interrupts on the Timer32_0 capture 0 pin, create a running average over the course of 200 milliseconds
*
*/

void TIMER32_0_IRQHandler(void){
	Chip_TIMER_Reset(LPC_TIMER32_0);					// Reset the timer immediately 
        Chip_TIMER_ClearCapture(LPC_TIMER32_0, 0);				// Clear the capture
	time = (time*count+Chip_TIMER_ReadCapture(LPC_TIMER32_0,0))/(1+count);	// Continue the running average 
	count++;								// Increase the count hto allow the running average to be properly computed
}



int main(void){
	SystemCoreClockUpdate();

	if(SysTick_Config(SystemCoreClock/1000)){
		while(true);
	}
	count = 0;

	// GPIO Initialization
	Chip_GPIO_Init(LPC_GPIO);					// Initialize GPIO for specific memory location
	Chip_GPIO_WriteDirBit(LPC_GPIO, TEST_PORT, TEST_PIN, true);	// Test for running code LED
	Chip_GPIO_SetPinState(LPC_GPIO, TEST_PORT, TEST_PIN, false);	// Test for running code LED
	
	// UART Setup
	Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO1_6, (IOCON_FUNC1 | IOCON_MODE_INACT));/* RXD */
        Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO1_7, (IOCON_FUNC1 | IOCON_MODE_INACT));/* TXD */

	Chip_UART_Init(LPC_USART);
        Chip_UART_SetBaud(LPC_USART, 57600);
        Chip_UART_ConfigData(LPC_USART, (UART_LCR_WLEN8 | UART_LCR_SBS_1BIT | UART_LCR_PARITY_DIS));
        Chip_UART_SetupFIFOS(LPC_USART, (UART_FCR_FIFO_EN | UART_FCR_TRG_LEV2));
        Chip_UART_TXEnable(LPC_USART);
	// End of UART setup

	// Timer Initialization things
	Chip_TIMER_Init(LPC_TIMER32_0);			// Initialize the timer 
	Chip_TIMER_Reset(LPC_TIMER32_0);		// Reset the timer
	Chip_TIMER_PrescaleSet(LPC_TIMER32_0, 0);	// Ensure that the prescaler is 0
	LPC_TIMER32_0->CCR |= 5;			// Set the first and third bits of the capture 

	// Interrupt setup things
	Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO1_5, (IOCON_FUNC2|IOCON_MODE_INACT));	// Set port 1, pin 5, to the 32 bit timer capture function, if using timer 1, set to digital mode
	NVIC_SetPriority(SysTick_IRQn, 1);						// Give the SysTick function a lower priority
	NVIC_SetPriority(TIMER_32_0_IRQn, 0);						// Ensure that the 32 bit timer capture interrupt has the highest priority
	NVIC_ClearPendingIRQ(TIMER_32_0_IRQn);						// Ensure that there are no pending interrupts on TIMER_32_0_IRQn
        NVIC_EnableIRQ(TIMER_32_0_IRQn);						// Enable interrupts on TIMER_32_0_IRQn

	Chip_TIMER_Enable(LPC_TIMER32_0);						// Start the timer
	
	lastPrint = msTicks;								// Give lastPrint an initial value of the current time


	while(1){	
			// For the rest of the time running...

		if(lastPrint+200<msTicks){	// 5 times per second
			itoa(SystemCoreClock/time, str, 10);			// Convert the frequency to a string of base 10 numbers
			Chip_UART_SendBlocking(LPC_USART, str, strlen(str));	// Send the frequency over UART to the computer
	       		Chip_UART_SendBlocking(LPC_USART, "\n\r", 2);		// Send a new line to ensure readability for the user
			lastPrint = msTicks;					// Store the current time, to allow the process to be done in another 1/5 second
			time = 0;						// Set the average time back to 0
			count = 0;						// Set the count for the average back to 0
		}

		Chip_GPIO_SetPinState(LPC_GPIO, TEST_PORT, TEST_PIN, true);			// Turn on the LED on port 2, pin 10 to ensure that the code made it into the loop
	}
}
