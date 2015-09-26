

#include "chip.h"
#include "util.h"

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/

#define TEST_CCAN_BAUD_RATE 500000

#define LED_PORT 2
#define LED_PIN 10

#define MINIMUM_HEARTBEAT_FREQ 10
#define TIMER_32_0_MATCH_SLOT 10

#define BUFFER_SIZE 8

const uint32_t OscRateIn = 12000000;

CCAN_MSG_OBJ_T msg_obj;

volatile uint32_t msTicks;
static volatile bool should_send_heartbeat = false;

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

void baudrateCalculate(uint32_t baud_rate, uint32_t *can_api_timing_cfg) {
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
	LED_On();
	/* Determine which CAN message has been received */
	msg_obj.msgobj = msg_obj_num;
	/* Now load up the msg_obj structure with the CAN message */
	LPC_CCAN_API->can_receive(&msg_obj);
	if (msg_obj_num == 3) {
		RingBuffer_Insert(&rx_buffer, &msg_obj);
	}
}

/*	CAN transmit callback */
/*	Function is executed by the Callback handler after
    a CAN message has been transmitted */
void CAN_tx(uint8_t msg_obj_num) {}

/*	CAN error callback */
/*	Function is executed by the Callback handler after
    an error has occured on the CAN bus */
void CAN_error(uint32_t error_info) {}

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

/**
 * @brief   (Heartbeat) Timer callback run every time a timer match hits
 * @return  Nothing
 */
void TIMER_34IRQHandler(){
	if (Chip_TIMER_MatchPending(LPC_TIMER32_0, 0)) {
		Chip_TIMER_ClearMatch(LPC_TIMER32_0, 0);
		should_send_heartbeat = true;
	}
}

/*
 * @brief Finds number of ticks after which we should send a message (have an interrupt)
 * @return 8-bit integer with tick period length
 */
uint8_t hertz2ticks(uint8_t minimum_heartbeat_freq, uint32_t core_clock_speed) {
    return core_clock_speed / minimum_heartbeat_freq;
}

void init_heartbeat_timer(LPC_TIMER_T *timer, uint8_t match_slot, uint8_t timer_interrupt_number, uint32_t core_clock_speed, uint8_t minimum_heartbeat_freq) {
	Chip_TIMER_Init(timer);
	Chip_TIMER_Reset(timer);
	Chip_TIMER_MatchEnableInt(timer, match_slot);
	Chip_TIMER_SetMatch(timer, match_slot, hertz2ticks(minimum_heartbeat_freq, core_clock_speed));
	Chip_TIMER_ResetOnMatchEnable(timer, match_slot);

	NVIC_ClearPendingIRQ(timer_interrupt_number);
	NVIC_EnableIRQ(timer_interrupt_number);
}

int main(void) {
    // Set correct register settings to initialize the system clock 
	SystemCoreClockUpdate();
    uint8_t freq_tick_interrupts = SystemCoreClock / 1000;
	if (SysTick_Config (freq_tick_interrupts)) {
		// Error in system clock!
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

    init_heartbeat_timer(LPC_TIMER32_0, TIMER_32_0_MATCH_SLOT, TIMER_32_0_IRQn, SystemCoreClock, MINIMUM_HEARTBEAT_FREQ);
	
	/* Configure message object 1 to receive all 11-bit messages */
	msg_obj.msgobj = 3;
	msg_obj.mode_id = 0x444;
	msg_obj.mask = 0xfff;
	LPC_CCAN_API->config_rxmsgobj(&msg_obj);

	LED_Off();

	while (1) {

        uint64_t ID = 0x1AF;

		if (should_send_heartbeat) {
			should_send_heartbeat = false;
            msg_obj.msgobj = 4;
            msg_obj.mode_id = ID;
            msg_obj.dlc = 1;
            msg_obj.data[1] = 1;

            LPC_CCAN_API->can_transmit(&msg_obj);
            DEBUG_Print("Sent heartbeat\n\r");
		}

		if (!RingBuffer_IsEmpty(&rx_buffer)) {
			CCAN_MSG_OBJ_T temp_msg;
			RingBuffer_Pop(&rx_buffer, &temp_msg);
			DEBUG_Print("Received Message\n\r");
			
			if(temp_msg.data[1] & 0x08){
				LED_On();
			} else{
				LED_Off();
			}
        }

		uint8_t count;
		if ((count = Chip_UART_Read(LPC_USART, Rx_Buf, 8)) != 0){
			Chip_UART_SendBlocking(LPC_USART, Rx_Buf, count);
			if (Rx_Buf[0] =='a') {
				msg_obj.msgobj = 4;
				msg_obj.mode_id = 0x1AF;
				msg_obj.dlc = 2;
				msg_obj.data[1] = 1;
				msg_obj.data[2] = 0x08;

				LPC_CCAN_API->can_transmit(&msg_obj);
				DEBUG_Print("Turned LED On\n\r");
			}
			else if (Rx_Buf[0] =='b') {
				msg_obj.msgobj = 4;
				msg_obj.mode_id = 0x1AF;
				msg_obj.dlc = 2;
				msg_obj.data[1] = 8;
				msg_obj.data[2] = 0;

				LPC_CCAN_API->can_transmit(&msg_obj);
				DEBUG_Print("Turned LED Off\n\r");
			}
		}
	}
}
