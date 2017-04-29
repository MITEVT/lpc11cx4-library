#ifndef _LEGACY_
#include "chip.h"
#include <string.h>
#include "can.h"

#define CAN_BUF_SIZE 16

CCAN_MSG_OBJ_T msg_obj;
static RINGBUFF_T rx_buffer;
CCAN_MSG_OBJ_T _rx_buffer[CAN_BUF_SIZE];

static RINGBUFF_T tx_buffer;
CCAN_MSG_OBJ_T _tx_buffer[CAN_BUF_SIZE];
CCAN_MSG_OBJ_T tmp_msg_obj; // temporarily store data/CAN ID to insert into tx_buf
CCAN_MSG_OBJ_T tmp_msg_obj_2;

static bool can_error_flag;
static uint32_t can_error_info;


#define NUM_MSG_OBJS 2
#if NUM_MSG_OBJS > 2
	#error "Allocating too many message objects to CAN Tx can cause bad shit"
#endif
static bool msg_obj_stat[NUM_MSG_OBJS + 1];


bool CAN_IsTxBusy(void);
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

uint8_t CAN_GetTxErrorCount(void) {
    // page 290 in the user manual
    return LPC_CCAN->CANEC & 0x000000FF;
}

uint8_t CAN_GetRxErrorCount(void) {
    // page 290 in the user manual
    return (LPC_CCAN->CANEC & 0x00007F00) >> 8;
}

bool CAN_IsTxBusy(void) {
    // page 302 in the user manual
    return ((LPC_CCAN->CANTXREQ1 & 0x00000002) >> 1) == 1;
}

// TODO SAVE CURRENT IN FLIGHT MESSAGE SO THAT ON RESET ANY IN FLIGHT MESSAGES CAN BE RE-SENT

void CAN_ResetPeripheral(void) {
    Chip_SYSCTL_PeriphReset(RESET_CAN0);
}

CAN_ERROR_T Convert_To_CAN_Error(uint32_t can_error) {
	if (!can_error) return NO_CAN_ERROR;
    switch(can_error & 0x5) {
       case 0x1: return STUF_CAN_ERROR;
       case 0x2: return FORM_CAN_ERROR;
       case 0x3: return ACK_CAN_ERROR;
       case 0x4: return BIT1_CAN_ERROR;
       case 0x5: return BIT0_CAN_ERROR;
       case 0x6: return CRC_CAN_ERROR;
       case 0x7: return UNUSED_CAN_ERROR;
    }
    if (can_error == 0x400) return UNRECOGNIZED_MSGOBJ_CAN_ERROR;
    if (can_error == 0x800) return RX_BUFFER_FULL_CAN_ERROR;
    return UNRECOGNIZED_ERROR_CODE;
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
	if (!RingBuffer_Insert(&rx_buffer, &msg_obj)) {
		can_error_flag = true;
		can_error_info = 0x800;
	}
}

/*	CAN transmit callback */
void CAN_tx(uint8_t msg_obj_num) {
	if (msg_obj_num <= NUM_MSG_OBJS) {
		msg_obj_stat[msg_obj_num] = false;

		if (RingBuffer_Pop(&tx_buffer, &tmp_msg_obj_2)){
			tmp_msg_obj_2.msgobj = msg_obj_num;
			LPC_CCAN_API->can_transmit(&tmp_msg_obj_2);
			msg_obj_stat[msg_obj_num] = true;
		}
	} else {
		can_error_flag = true;
		can_error_info = 0x400;
	}
}

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

	RingBuffer_Init(&tx_buffer, _tx_buffer, sizeof(CCAN_MSG_OBJ_T), CAN_BUF_SIZE);
	RingBuffer_Flush(&tx_buffer);

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

	/* Configure message objects to accept all messages */
	CAN_SetMask1(0, 0);
	CAN_SetMask2(0, 0);

	can_error_flag = false;
	can_error_info = 0;

	memset(msg_obj_stat, 0, sizeof(bool)*NUM_MSG_OBJS);
}

// ANDs the mask with the input ID and checks if == to mode_id
void CAN_SetMask1(uint32_t mask, uint32_t mode_id) {
	msg_obj.mode_id = mode_id;
	msg_obj.mask = mask; 
	msg_obj.msgobj = 27;
	LPC_CCAN_API->config_rxmsgobj(&msg_obj);
	msg_obj.msgobj = 28;
	LPC_CCAN_API->config_rxmsgobj(&msg_obj);
	msg_obj.msgobj = 29;
	LPC_CCAN_API->config_rxmsgobj(&msg_obj);
}

void CAN_SetMask2(uint32_t mask, uint32_t mode_id) {
	msg_obj.mode_id = mode_id;
	msg_obj.mask = mask; 
	msg_obj.msgobj = 30;
	LPC_CCAN_API->config_rxmsgobj(&msg_obj);
	msg_obj.msgobj = 31;
	LPC_CCAN_API->config_rxmsgobj(&msg_obj);
}

CAN_ERROR_T CAN_Receive(CCAN_MSG_OBJ_T* user_buffer) {
	if (can_error_flag) {
		can_error_flag = false;
		return Convert_To_CAN_Error(can_error_info);
	} else if (RingBuffer_Pop(&rx_buffer, user_buffer)) {
	    return NO_CAN_ERROR;
	}
	return NO_RX_CAN_MESSAGE;
}

CAN_ERROR_T CAN_Transmit(uint32_t msg_id, uint8_t* data, uint8_t data_len) {
	tmp_msg_obj.mode_id = msg_id;
	tmp_msg_obj.dlc = data_len;
	uint8_t i;
	for (i = 0; i < tmp_msg_obj.dlc; i++) {	
		tmp_msg_obj.data[i] = data[i];
	}
	return CAN_TransmitMsgObj(&tmp_msg_obj);
}
 // TODO clear can error info after getting error
CAN_ERROR_T CAN_TransmitMsgObj(CCAN_MSG_OBJ_T *msg_obj) {
	if (can_error_flag && (can_error_info != 0x800)) { // Not rx buffer full error
		can_error_flag = false;
		return Convert_To_CAN_Error(can_error_info);
	} else {
		uint8_t i;
		bool sent = false;
		for (i = 1; i <= NUM_MSG_OBJS; i++) {
			if (!msg_obj_stat[i]) { // Message Object is free, begin to send
				// Send with this message object
				msg_obj->msgobj = i;
				LPC_CCAN_API->can_transmit(msg_obj);
				msg_obj_stat[i] = true;
				sent = true;
				break;
			}
		}

		if (!sent) { // Everything is busy, so put in ring buffer
			if (!RingBuffer_Insert(&tx_buffer, msg_obj)) {
				return TX_BUFFER_FULL_CAN_ERROR;
			}
		}

	    return NO_CAN_ERROR;
	}
}

uint8_t str[5];

void CAN_PrintStatus(void) {
	Chip_UART_SendBlocking(LPC_USART, (msg_obj_stat[0] ? "1 " : "0 "), 2);
	Chip_UART_SendBlocking(LPC_USART, (msg_obj_stat[0] ? "1\n" : "0\n"), 2);

	itoa(RingBuffer_GetCount(&tx_buffer), str, 10);
	Chip_UART_SendBlocking(LPC_USART, str, 2);
	Chip_UART_SendBlocking(LPC_USART, " ", 1);
	itoa(RingBuffer_GetCount(&rx_buffer), str, 10);
	Chip_UART_SendBlocking(LPC_USART, str, 2);
	Chip_UART_SendBlocking(LPC_USART, "\r\n", 1);

}

CAN_ERROR_T CAN_GetErrorStatus(void) {
	return Convert_To_CAN_Error(can_error_info);
}

#endif
