#include "chip.h"
#include "util.h"
#include <string.h>

#define TEST_CCAN_BAUD_RATE 500000

#define TEST_PORT 2
#define TEST_PIN 10

#define DEBUG_ENABLE

#ifdef DEBUG_ENABLE
	#define DEBUG_Print(str) Chip_UART_SendBlocking(LPC_USART, str, strlen(str))
	#define DEBUG_Write(str, count) Chip_UART_SendBlocking(LPC_USART, str, count)
#else
	#define DEBUG_Print(str)
	#define DEBUG_Write(str, count) 
#endif

/*
Times the interrupts of on Pin 9
*/

const uint32_t OscRateIn = 12000000;

CCAN_MSG_OBJ_T msg_obj;

volatile uint32_t msTicks;

static char str[20];		// The array of chars for UART output

static volatile uint32_t time;	// The current running average fo time
static volatile uint32_t count;	// The number of samples taken since the last output
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

void canBaudrateCalculate(uint32_t baud_rate, uint32_t *can_api_timing_cfg) {
	uint32_t pClk, div, quanta, segs, seg1, seg2, clk_per_bit, can_sjw;
	Chip_Clock_EnablePeriphClock(SYSCTL_CLOCK_CAN);
	pClk = Chip_Clock_GetMainClockRate();

	clk_per_bit = pClk / baud_rate;

	for (div = 0; div <= 15; div++) {
		for (quanta = 1; quanta <= 32; quanta++) {
			for (segs = 3; segs <= 17; segs++) {
				if (clk_per_bit == (segs * quanta * (div + 1))) {
					segs -= 3;
					seg1 = segs / 2;
					seg2 = segs - seg1;
					can_sjw = seg1 > 3 ? 3 : seg1;
					can_api_timing_cfg[0] = div;
					can_api_timing_cfg[1] =
						((quanta - 1) & 0x3F) | (can_sjw & 0x03) << 6 | (seg1 & 0x0F) << 8 | (seg2 & 0x07) << 12;
					return;
				}
			}
		}
	}
}

/*	CAN receive callback */
/*	Function is executed by the Callback handler after
    a CAN message has been received */
void CAN_rx(uint8_t msg_obj_num) {
}


volatile bool t = false;
/*	CAN transmit callback */
/*	Function is executed by the Callback handler after
    a CAN message has been transmitted */
void CAN_tx(uint8_t msg_obj_num) {
	if (msg_obj_num == 0) t = true;
}


static volatile bool b = false;
static volatile uint32_t can_error = 0;
/*	CAN error callback */
/*	Function is executed by the Callback handler after
    an error has occurred on the CAN bus */
void CAN_error(uint32_t error_info) {
	b = true;
	can_error = error_info;
}

/**
 * @brief	CCAN Interrupt Handler
 * @return	Nothing
 * @note	The CCAN interrupt handler must be provided by the user application.
 *	It's function is to call the isr() API located in the ROM
 */
void CAN_IRQHandler(void) {
	LPC_CCAN_API->isr();
}

uint8_t Rx_Buf[8];

uint32_t lastError = 0;

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
	Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO1_5, (IOCON_FUNC2|IOCON_MODE_INACT));	// Set port 1, pin 5, to the 32 bit timer capture function
	NVIC_SetPriority(SysTick_IRQn, 1);						// Give the SysTick function a lower priority
	NVIC_SetPriority(TIMER_32_0_IRQn, 0);						// Ensure that the 32 bit timer capture interrupt has the highest priority
	NVIC_ClearPendingIRQ(TIMER_32_0_IRQn);						// Ensure that there are no pending interrupts on TIMER_32_0_IRQn
	NVIC_SetPriority(CAN_IRQn, 2);
    NVIC_EnableIRQ(TIMER_32_0_IRQn);						// Enable interrupts on TIMER_32_0_IRQn

	//Chip_TIMER_Enable(LPC_TIMER32_0);						// Start the timer
	
	lastPrint = msTicks;								// Give lastPrint an initial value of the current time

	//CAN stuff hopefully
	uint32_t CanApiClkInitTable[2];
	/* Publish CAN Callback Functions */
	CCAN_CALLBACKS_T callbacks = {
		CAN_rx,
		CAN_tx,
		CAN_error,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
	};
	canBaudrateCalculate(TEST_CCAN_BAUD_RATE, CanApiClkInitTable);

	LPC_CCAN_API->init_can(CanApiClkInitTable, TRUE);
	/* Configure the CAN callback functions */
	LPC_CCAN_API->config_calb(&callbacks);
	/* Enable the CAN Interrupt */
	NVIC_EnableIRQ(CAN_IRQn);

	Chip_UART_SendBlocking(LPC_USART, "=====\r\n", 7);

	msg_obj.msgobj = 1;
	msg_obj.mode_id = 0x000;
	msg_obj.mask = 0x000;
	LPC_CCAN_API->config_rxmsgobj(&msg_obj);

	b = false;
	t = false;

	while(1){	
		// For the rest of the time running...

		if(msTicks - lastPrint > 1000){	// 5 times per second
			// itoa(SystemCoreClock/time, str, 10);			// Convert the frequency to a string of base 10 numbers
			// Chip_UART_SendBlocking(LPC_USART, str, strlen(str));	// Send the frequency over UART to the computer
	  //      	Chip_UART_SendBlocking(LPC_USART, "\n\r", 2);		// Send a new line to ensure readability for the user
			lastPrint = msTicks;					// Store the current time, to allow the process to be done in another 1/5 second

			msg_obj.msgobj = 0;
			msg_obj.mode_id = 0x100;
			msg_obj.dlc = 2;
			// msg_obj.data[0] = (uint8_t)((60*(SystemCoreClock/time)/47) & 0xFF);
			// msg_obj.data[1] = (uint8_t)((60*(SystemCoreClock/time)/47) & 0xFF00 >> 8);
			msg_obj.dlc = 0;
			msg_obj.data[0] = 0xAA;
			msg_obj.data[1] = 0xAA;

			time = 0;						// Set the average time back to 0
			count = 0;						// Set the count for the average back to 0

			DEBUG_Print("Trying to send...\r\n");
			LPC_CCAN_API->can_transmit(&msg_obj);
		}

		if (b) {
			b = false;
			Chip_UART_SendBlocking(LPC_USART, "Error: ", 7);
			itoa(can_error, str, 2);
			Chip_UART_SendBlocking(LPC_USART, str, strlen(str));
			Chip_UART_SendBlocking(LPC_USART, "\r\n", 2);
			// lastError = msTicks;
		}

		if (t) {
			t = false;
			DEBUG_Print("\tSuccessfully Sent\r\n");
		}

		Chip_GPIO_SetPinState(LPC_GPIO, TEST_PORT, TEST_PIN, true);			// Turn on the LED on port 2, pin 10 to ensure that the code made it into the loop
	}
}
