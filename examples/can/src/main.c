/*
 * @brief CCAN on-chip driver example
 *
 * @note
 * Copyright(C) NXP Semiconductors, 2012
 * All rights reserved.
 *
 * @par
 * Software that is described herein is for illustrative purposes only
 * which provides customers with programming information regarding the
 * LPC products.  This software is supplied "AS IS" without any warranties of
 * any kind, and NXP Semiconductors and its licensor disclaim any and
 * all warranties, express or implied, including all implied warranties of
 * merchantability, fitness for a particular purpose and non-infringement of
 * intellectual property rights.  NXP Semiconductors assumes no responsibility
 * or liability for the use of the software, conveys no license or rights under any
 * patent, copyright, mask work right, or any other intellectual property rights in
 * or to any products. NXP Semiconductors reserves the right to make changes
 * in the software without notification. NXP Semiconductors also makes no
 * representation or warranty that such application will be suitable for the
 * specified use without further testing or modification.
 *
 * @par
 * Permission to use, copy, modify, and distribute this software and its
 * documentation is hereby granted, under NXP Semiconductors' and its
 * licensor's relevant copyrights in the software, without fee, provided that it
 * is used in conjunction with NXP Semiconductors microcontrollers.  This
 * copyright, permission, and disclaimer notice must appear in all copies of
 * this code.
 */

#include "chip.h"

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/

#define TEST_CCAN_BAUD_RATE 500000

#define LED_PIN 7
#define LED_PIN2 0

const uint32_t OscRateIn = 12000000;

CCAN_MSG_OBJ_T msg_obj;

uint8_t message_received = 0;
uint32_t id;
uint8_t data[8];
uint8_t dlc;

volatile uint32_t msTicks;

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/

/*****************************************************************************
 * Private functions
 ****************************************************************************/

void SysTick_Handler(void) {
	msTicks++;
}

static void Delay(uint32_t dlyTicks) {
	uint32_t curTicks = msTicks;
	while ((msTicks - curTicks) < dlyTicks);
}

void reverse(char str[], int length)
{
    int start = 0;
    int end = length -1;
    while (start < end)
    {
    	char t = *(str+start);
    	*(str+start) = *(str+end);
    	*(str+end) = t;
        start++;
        end--;
    }
}
 
// Implementation of itoa()
char* itoa(int num, char* str, int base)
{
    int i = 0;
    bool isNegative = false;
 
    /* Handle 0 explicitely, otherwise empty string is printed for 0 */
    if (num == 0)
    {
        str[i++] = '0';
        str[i] = '\0';
        return str;
    }
 
    // In standard itoa(), negative numbers are handled only with 
    // base 10. Otherwise numbers are considered unsigned.
    if (num < 0 && base == 10)
    {
        isNegative = true;
        num = -num;
    }
 
    // Process individual digits
    while (num != 0)
    {
        int rem = num % base;
        str[i++] = (rem > 9)? (rem-10) + 'a' : rem + '0';
        num = num/base;
    }
 
    // If number is negative, append '-'
    if (isNegative)
        str[i++] = '-';
 
    str[i] = '\0'; // Append string terminator
 
    // Reverse the string
    reverse(str, i);
 
    return str;
}


static void GPIO_Config(void) {
	Chip_GPIO_Init(LPC_GPIO);
}

static void LED_Config(void) {
	Chip_GPIO_WriteDirBit(LPC_GPIO, 0, LED_PIN, true);
	Chip_GPIO_WriteDirBit(LPC_GPIO, 3, LED_PIN2, true);
}

static void LED_On(void) {
	Chip_GPIO_SetPinState(LPC_GPIO, 0, LED_PIN, true);
}

static void LED2_On(void) {
	Chip_GPIO_SetPinState(LPC_GPIO, 3, LED_PIN2, true);
}

static void LED_Off(void) {
	Chip_GPIO_SetPinState(LPC_GPIO, 0, LED_PIN, false);
}

static void LED2_Off(void) {
	Chip_GPIO_SetPinState(LPC_GPIO, 3, LED_PIN2, false);
}


void baudrateCalculate(uint32_t baud_rate, uint32_t *can_api_timing_cfg)
{
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
	// LED_On();
	/* Determine which CAN message has been received */
	msg_obj.msgobj = msg_obj_num;

	/* Now load up the msg_obj structure with the CAN message */
	LPC_CCAN_API->can_receive(&msg_obj);
	char num[2];
	itoa((int)msg_obj_num, num, 10);
	// int i;
	// for(i = 0; i < 0xFF; i++);
	// LED_Off();
	// if (msg_obj_num == 1) {
	// 	/* Simply transmit CAN frame (echo) with with ID +0x100 via buffer 2 */
	// 	// msg_obj.msgobj = 2;
	// 	// msg_obj.mode_id += 0x100;
	// 	// LPC_CCAN_API->can_transmit(&msg_obj);
	// 	id = msg_obj.mode_id;
	// 	data[8] = msg_obj.data;
	// 	dlc = msg_obj.dlc;
	// 	message_received = 1;
	// } 
	Chip_UART_SendBlocking(LPC_USART, "Message Re\n", 11);
	// Delay(1);
	Chip_UART_SendBlocking(LPC_USART, num, 2);

	char *s = "Passed Delay\n";
	Chip_UART_SendBlocking(LPC_USART, s, 14);
	msg_obj.msgobj = 2;
	msg_obj.mode_id += 0x100;
	LPC_CCAN_API->can_transmit(&msg_obj);
	s = "Message Sent\n";
	Chip_UART_SendBlocking(LPC_USART, s, 17);
}

/*	CAN transmit callback */
/*	Function is executed by the Callback handler after
    a CAN message has been transmitted */
void CAN_tx(uint8_t msg_obj_num) {
	char *t = "T\n";
	Chip_UART_SendBlocking(LPC_USART, t, 3);
}


char *error = "Error\n";
/*	CAN error callback */
/*	Function is executed by the Callback handler after
    an error has occured on the CAN bus */
void CAN_error(uint32_t error_info) {
	LED2_On();
	Chip_UART_SendBlocking(LPC_USART, error, 7);

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

/*****************************************************************************
 * Public functions
 ****************************************************************************/

/**
 * @brief	Main routine for CCAN_ROM example
 * @return	Nothing
 */
int main(void)
{

	SystemCoreClockUpdate();

	if (SysTick_Config (SystemCoreClock / 1000)) {
		//Error
		while(1);
	}

	GPIO_Config();
	LED_Config();

	//---------------
	//UART
	Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO1_6, (IOCON_FUNC1 | IOCON_MODE_INACT));/* RXD */
	Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO1_7, (IOCON_FUNC1 | IOCON_MODE_INACT));/* TXD */

	Chip_UART_Init(LPC_USART);
	Chip_UART_SetBaud(LPC_USART, 9600);
	Chip_UART_ConfigData(LPC_USART, (UART_LCR_WLEN8 | UART_LCR_SBS_1BIT | UART_LCR_PARITY_DIS));
	Chip_UART_SetupFIFOS(LPC_USART, (UART_FCR_FIFO_EN | UART_FCR_TRG_LEV2));
	Chip_UART_TXEnable(LPC_USART);
	//---------------

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
	SystemCoreClockUpdate();
	baudrateCalculate(TEST_CCAN_BAUD_RATE, CanApiClkInitTable);

	LPC_CCAN_API->init_can(&CanApiClkInitTable[0], TRUE);
	/* Configure the CAN callback functions */
	LPC_CCAN_API->config_calb(&callbacks);
	/* Enable the CAN Interrupt */
	NVIC_EnableIRQ(CAN_IRQn);

	/* Send a simple one time CAN message */
	// msg_obj.msgobj  = 0;
	// msg_obj.mode_id = 0x145;
	// msg_obj.mask    = 0x0;
	// msg_obj.dlc     = 4;
	// msg_obj.data[0] = 'T';	// 0x54
	// msg_obj.data[1] = 'E';	// 0x45
	// msg_obj.data[2] = 'S';	// 0x53
	// msg_obj.data[3] = 'T';	// 0x54
	// LPC_CCAN_API->can_transmit(&msg_obj);

	/* Configure message object 1 to receive all 11-bit messages 0x050-0x050 */
	msg_obj.msgobj = 1;
	msg_obj.mode_id = 0x050;
	msg_obj.mask = 0xFFF;
	LPC_CCAN_API->config_rxmsgobj(&msg_obj);

	char *space = " ";
	char *p = "Message Received\n";

	LED_Off();
	LED2_Off();

	char *startMessage = "Started Up\n";
	Chip_UART_SendBlocking(LPC_USART, startMessage, 12);
	while (1) {



		//__WFI();	/* Go to Sleep */
		// if (message_received) {
		// 	message_received = 0;
		// 	msg_obj.msgobj  = 0;
		// 	msg_obj.mode_id = 0x145;
		// 	msg_obj.mask    = 0x0;
		// 	msg_obj.dlc     = 4;
		// 	msg_obj.data[0] = 'T';	// 0x54
		// 	msg_obj.data[1] = 'E';	// 0x45
		// 	msg_obj.data[2] = 'S';	// 0x53
		// 	msg_obj.data[3] = 'T';	// 0x54
		// 	LPC_CCAN_API->can_transmit(&msg_obj);
		// }
		
	}
}
