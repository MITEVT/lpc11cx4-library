#ifndef _CAN_H_
#define _CAN_H_

typedef enum CAN_ERROR {
    NO_CAN_ERROR,
    NO_RX_CAN_MESSAGE,
} CAN_ERROR_T;

void CAN_Init(uint32_t baud_rate);
CAN_ERROR_T CAN_Receive(CCAN_MSG_OBJ_T* user_buffer);
CAN_ERROR_T CAN_Transmit(uint8_t* data, uint32_t msg_id);
CAN_ERROR_T CAN_GetErrorStatus(void);

#endif
