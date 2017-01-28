#ifndef _CAN_H_
#define _CAN_H_

void CAN_Init(uint32_t baud_rate);
uint32_t CAN_Receive(CCAN_MSG_OBJ_T* user_buffer);
uint32_t CAN_Transmit(uint8_t* data, uint32_t msg_id);
uint32_t CAN_GetErrorStatus(void);

#endif
