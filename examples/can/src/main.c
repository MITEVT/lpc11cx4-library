
#include "stepperMotor.h"
#include "chip.h"
#include "util.h"
#include "string.h"

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/

#define TEST_CCAN_BAUD_RATE 500000

#define LED_PORT 2
#define LED_PIN 10

#define BUFFER_SIZE 8

const uint32_t OscRateIn = 12000000;

CCAN_MSG_OBJ_T msg_obj;

volatile uint32_t msTicks;

#define Hertz2Ticks(freq) SystemCoreClock / freq

STATIC RINGBUFF_T rx_buffer;
CCAN_MSG_OBJ_T _rx_buffer[8];

static char str[100];

#define DEBUG_ENABLE

#ifdef DEBUG_ENABLE
	#define DEBUG_Print(str) Chip_UART_SendBlocking(LPC_USART, str, strlen(str))
	#define DEBUG_Write(str, count) Chip_UART_SendBlocking(LPC_USART, str, count)
#else
	#define DEBUG_Print(str)
	#define DEBUG_Write(str, count) 
#endif


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

static void GPIO_Config(void) {
	Chip_GPIO_Init(LPC_GPIO);
}

static void LED_Config(void) {
	Chip_GPIO_WriteDirBit(LPC_GPIO, LED_PORT, LED_PIN, true);
}

static void LED_On(void) {
	Chip_GPIO_SetPinState(LPC_GPIO, LED_PORT, LED_PIN, true);
}

static void LED_Off(void) {
	Chip_GPIO_SetPinState(LPC_GPIO, LED_PORT, LED_PIN, false);
}

//----

// static void LED2_Config(void) {
// 	Chip_GPIO_WriteDirBit(LPC_GPIO, LED2_PORT, LED2_PIN, true);
// }

// static void LED2_On(void) {
// 	Chip_GPIO_SetPinState(LPC_GPIO, LED2_PORT, LED2_PIN, true);
// }

// static void LED2_Off(void) {
// 	Chip_GPIO_SetPinState(LPC_GPIO, LED2_PORT, LED2_PIN, false);
// }

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
	if (msg_obj_num == 1) {
		RingBuffer_Insert(&rx_buffer, &msg_obj);
	}
}

/*	CAN transmit callback */
/*	Function is executed by the Callback handler after
    a CAN message has been transmitted */
void CAN_tx(uint8_t msg_obj_num) {}


bool b = false;
/*	CAN error callback */
/*	Function is executed by the Callback handler after
    an error has occured on the CAN bus */
void CAN_error(uint32_t error_info) {

	b = true;
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

bool sent = false;
bool isAlive = true;

void TIMER32_0_IRQHandler(void) {
	if (Chip_TIMER_MatchPending(LPC_TIMER32_0, 0)) {
		Chip_TIMER_ClearMatch(LPC_TIMER32_0, 0);
		if(!isAlive)
			LED_On();
		isAlive = false;
		sent = true;

	}
}

uint8_t Rx_Buf[8];

int main(void)
{

	SystemCoreClockUpdate();

	if (SysTick_Config (SystemCoreClock / 1000)) {
		//Error
		while(1);
	}

	GPIO_Config();
	LED_Config();

	Chip_TIMER_Init(LPC_TIMER32_0);
	Chip_TIMER_Reset(LPC_TIMER32_0);
	Chip_TIMER_MatchEnableInt(LPC_TIMER32_0, 0);
	Chip_TIMER_SetMatch(LPC_TIMER32_0, 0, SystemCoreClock/2);
	Chip_TIMER_ResetOnMatchEnable(LPC_TIMER32_0, 0);

	Chip_TIMER_Enable(LPC_TIMER32_0);

	/* Enable timer interrupt */
	NVIC_ClearPendingIRQ(TIMER_32_0_IRQn);
	NVIC_EnableIRQ(TIMER_32_0_IRQn);

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

	DEBUG_Print("Started up\n\r");

	//---------------
	//Ring Buffer

	RingBuffer_Init(&rx_buffer, _rx_buffer, sizeof(CCAN_MSG_OBJ_T), 8);
	RingBuffer_Flush(&rx_buffer);

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
	baudrateCalculate(TEST_CCAN_BAUD_RATE, CanApiClkInitTable);

	LPC_CCAN_API->init_can(&CanApiClkInitTable[0], TRUE);
	/* Configure the CAN callback functions */
	LPC_CCAN_API->config_calb(&callbacks);
	/* Enable the CAN Interrupt */
	NVIC_EnableIRQ(CAN_IRQn);

    Chip_TIMER_Init(LPC_TIMER32_0);
	Chip_TIMER_Reset(LPC_TIMER32_0);
	Chip_TIMER_MatchEnableInt(LPC_TIMER32_0, 0);
	Chip_TIMER_SetMatch(LPC_TIMER32_0, 0, Hertz2Ticks(1));
	Chip_TIMER_ResetOnMatchEnable(LPC_TIMER32_0, 0);
	// Chip_TIMER_Enable(LPC_TIMER32_0);

	NVIC_ClearPendingIRQ(TIMER_32_0_IRQn);
	NVIC_EnableIRQ(TIMER_32_0_IRQn);

	// typedef struct CCAN_MSG_OBJ {
	// 	uint32_t  mode_id;
	// 	uint32_t  mask;
	// 	uint8_t   data[8];
	// 	uint8_t   dlc;
	// 	uint8_t   msgobj;
	// } CCAN_MSG_OBJ_T;

	/* Configure message object 1 to receive all 11-bit messages */
	msg_obj.msgobj = 1;
	msg_obj.mode_id = 0x600;
	msg_obj.mask = 0xFFF;
	LPC_CCAN_API->config_rxmsgobj(&msg_obj);

	LED_On();

	SystemCoreClockUpdate();


	if (SysTick_Config (SystemCoreClock / 1000)) {
		//Error
		while(1);
	}


	Stepper_Init(640);
	Stepper_ZeroPosition();
	Stepper_SetSpeed(46);
	Delay(150);

	while (1) {
		//__WFI();	/* Go to Sleep */
		if (!RingBuffer_IsEmpty(&rx_buffer)) {
			CCAN_MSG_OBJ_T temp_msg;
			RingBuffer_Pop(&rx_buffer, &temp_msg);
			DEBUG_Print("\r\nReceived Correct Message!\n\r");
			

			if (temp_msg.data[0] & 2){
				Stepper_Step(100);
				Delay(100);
				DEBUG_Print("\r\nfun!\n\r");
			}
				

			if (temp_msg.data[0] & 8){
				Stepper_HomePosition();
				Delay(100);	
			}

			if (temp_msg.data[0] & 4){	
				Stepper_HomePosition();
				Delay(100);
			}
			if (temp_msg.data[1] & 1){
				Stepper_Step(300);
				Stepper_ZeroPosition();
				
			}
			if (temp_msg.data[2] & 15){
				Stepper_ZeroPosition();
		

			// isAlive=true;

			// if (temp_msg.data[1] & 1){
			// 	LED_On();
			// 	DEBUG_Print("LED On \r\n");
			// }
			// if (temp_msg.data[1] & 2){
			// 	DEBUG_Print("Delay \r\n");
			// 	Delay(1000);
			// }
			// if (temp_msg.data[1] & 4){
			// 	DEBUG_Print("Hi\n\r");
			// }
			// if (temp_msg.data[1] & 8){
			// 	DEBUG_Print("LED Off\r\n");
			// 	LED_Off();
			}

		}	

		LED_On();
		Delay(500);
		LED_Off();
		Delay(500);

		if (b) {
			b = false;
			DEBUG_Print("CAN Error\r\n");
		}

		// if (sent) {
		// 	sent = false;
		// 	DEBUG_Print("Sent\r\n");
		// }

		uint8_t count;
		if ((count = Chip_UART_Read(LPC_USART, Rx_Buf, 8)) != 0) { // This If statement sends 
																	//count = the number fo available bytes and then cehcsk that it isn't zero
			Chip_UART_SendBlocking(LPC_USART, Rx_Buf, count); 		// Echo out user input (if you disable this then picocom will seem as if you aren't typeing anything)
			if (Rx_Buf[0] == '1') {
                msg_obj.msgobj = 2;
                msg_obj.mode_id = 0x601;
                msg_obj.dlc = 1;
                msg_obj.data[1] = 0x01;
                

                LPC_CCAN_API->can_transmit(&msg_obj);
                DEBUG_Print("\r\nSomething Should Happen\r\n");
            } 




            else if (Rx_Buf[0] == '2') {
            	msg_obj.msgobj = 2;
                msg_obj.mode_id = 0x1AF;
                msg_obj.dlc = 1;
                msg_obj.data[1] = 2;

                LPC_CCAN_API->can_transmit(&msg_obj);
                DEBUG_Print("\r\nLED Should Turn off\r\n");
            } else if (Rx_Buf[0] == 'c') {
            	msg_obj.msgobj = 2;
                msg_obj.mode_id = 0x600;
                msg_obj.dlc = 5;
                msg_obj.data[1] = 0xB0;
                msg_obj.data[2] = 0x0F;
                msg_obj.data[3] = 0;

                LPC_CCAN_API->can_transmit(&msg_obj);
                DEBUG_Print("\r\nHello World should be printed\r\n");
            } else if (Rx_Buf[0] == 'd') {
            	msg_obj.msgobj = 2;
                msg_obj.mode_id = 0x600;
                msg_obj.dlc = 5;
                msg_obj.data[1] = 0x07;
                msg_obj.data[2] = 0XB0;
                msg_obj.data[3] = 0x20;

                LPC_CCAN_API->can_transmit(&msg_obj);
                DEBUG_Print("\r\nShould recieve message\r\n");
            } else if (Rx_Buf[0] == 'e') {
            	msg_obj.msgobj = 2;
                msg_obj.mode_id = 0x600;
                msg_obj.dlc = 5;
                msg_obj.data[1] = 0;
                msg_obj.data[2] = 0x08;
                msg_obj.data[3] = 0x20;

                LPC_CCAN_API->can_transmit(&msg_obj);
                DEBUG_Print("\r\nShould recieve message\r\n");

            } else if (Rx_Buf[0] == 'z') {
            	msg_obj.msgobj = 2;
                msg_obj.mode_id = 0x601;
                msg_obj.dlc = 1;
                msg_obj.data[0] = 2;

                LPC_CCAN_API->can_transmit(&msg_obj);
                DEBUG_Print("\r\nLED Should Turn on\r\n");
			} else if (Rx_Buf[0] == 'x') {
            	msg_obj.msgobj = 2;
                msg_obj.mode_id = 0x601;
                msg_obj.dlc = 1;
                msg_obj.data[0] = 128;

                LPC_CCAN_API->can_transmit(&msg_obj);
                DEBUG_Print("\r\nLED Should Turn off\r\n");
        	}
		}
	}
}
