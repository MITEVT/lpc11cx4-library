#include "chip.h"
#include <string.h>
#include "can.h"

#define CAN_BUF_SIZE 8

CCAN_MSG_OBJ_T msg_obj;
STATIC RINGBUFF_T rx_buffer;
CCAN_MSG_OBJ_T _rx_buffer[CAN_BUF_SIZE];
static bool can_error_flag;
static uint32_t can_error_info;

void Baudrate_Calculate(uint32_t baud_rate, uint32_t *can_api_timing_cfg);
CAN_ERROR_T Convert_To_CAN_Error(uint32_t can_error);

/*************************************************
 *                  HELPERS
 * ************************************************/

// TODO EXPLAIN WHAT THIS DOES AND SIMPLIFY
void Baudrate_Calculate(uint32_t baud_rate, uint32_t *can_api_timing_cfg) {
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

CAN_ERROR_T Convert_To_CAN_Error(uint32_t can_error) {
    return can_error;
}

/*************************************************
 *                  CALLBACKS
 * ************************************************/

/*	CAN receive callback */
void CAN_rx(uint8_t msg_obj_num) {
	/* Determine which CAN message has been received */
	msg_obj.msgobj = msg_obj_num;
	/* Now load up the msg_obj structure with the CAN message */
	LPC_CCAN_API->can_receive(&msg_obj);
	RingBuffer_Insert(&rx_buffer, &msg_obj);
}

/*	CAN transmit callback */
void CAN_tx(uint8_t msg_obj_num) {}

/*	CAN error callback */
void CAN_error(uint32_t error_info) {
	can_error_info = error_info;
	can_error_flag = true;
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

void CAN_Init(uint32_t baud_rate) {

	RingBuffer_Init(&rx_buffer, _rx_buffer, sizeof(CCAN_MSG_OBJ_T), CAN_BUF_SIZE);
	RingBuffer_Flush(&rx_buffer);

	uint32_t CanApiClkInitTable[2];
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
	Baudrate_Calculate(baud_rate, CanApiClkInitTable);

	LPC_CCAN_API->init_can(&CanApiClkInitTable[0], TRUE);
	/* Configure the CAN callback functions */
	LPC_CCAN_API->config_calb(&callbacks);
	/* Enable the CAN Interrupt */
	NVIC_EnableIRQ(CAN_IRQn);

	/* Configure message object 1 to only ID 0x600 */
	msg_obj.msgobj = 1;
	msg_obj.mode_id = 0x600;
    // ANDs the mask with the input ID and checks if == to mode_id
	msg_obj.mask = 0x000; 
	LPC_CCAN_API->config_rxmsgobj(&msg_obj);

	can_error_flag = false;
	can_error_info = 0;
}

CAN_ERROR_T CAN_Receive(CCAN_MSG_OBJ_T* user_buffer) {
	if (can_error_flag) {
		can_error_flag = false;
		return Convert_To_CAN_Error(can_error_info);
	} else {
		if (!RingBuffer_IsEmpty(&rx_buffer)) {
			RingBuffer_Pop(&rx_buffer, user_buffer);
		} else {
            return NO_RX_CAN_MESSAGE;
        }
	}
}

CAN_ERROR_T CAN_Transmit(uint8_t* data, uint32_t msg_id) {
	if (can_error_flag) {
		can_error_flag = false;
		return Convert_To_CAN_Error(can_error_info);
	} else {
		msg_obj.msgobj = 2;
		msg_obj.mode_id = msg_id;
		msg_obj.dlc = sizeof(data) / sizeof(uint8_t);
		uint8_t i;
		for (i = 0; i < msg_obj.dlc; i++) {	
			msg_obj.data[i] = data[i];
		}
		LPC_CCAN_API->can_transmit(&msg_obj);
		return NO_CAN_ERROR;
	}
}

CAN_ERROR_T CAN_GetErrorStatus(void) {
	return Convert_To_CAN_Error(can_error_info);
}

