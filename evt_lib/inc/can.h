#ifndef _CAN_H_
#define _CAN_H_

typedef enum CAN_ERROR {
    NO_CAN_ERROR,
    NO_RX_CAN_MESSAGE,
} CAN_ERROR_T;

void CAN_Init(uint32_t baud_rate);
CAN_ERROR_T CAN_Receive(CCAN_MSG_OBJ_T* user_buffer);
CAN_ERROR_T CAN_Transmit(uint32_t msg_id, uint8_t* data, uint8_t data_len);
CAN_ERROR_T CAN_TransmitMsgObj(CCAN_MSG_OBJ_T *msg_obj);
CAN_ERROR_T CAN_GetErrorStatus(void);

uint8_t CAN_GetTxErrorCount(void);
uint8_t CAN_GetRxErrorCount(void);
bool CAN_IsTXBusy(void);
void CAN_ResetPeripheral(void);

#endif
