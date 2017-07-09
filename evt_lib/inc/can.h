#ifndef _LEGACY_
#ifndef _CAN_H_
#define _CAN_H_

// integrates with and based on ccand_11x.h error types
// explanations of page 289 of lpc11cx4 user manual
typedef enum CAN_ERROR {
    NO_CAN_ERROR,
    NO_RX_CAN_MESSAGE,
    EPASS_CAN_ERROR,
    WARN_CAN_ERROR_WARN,
    BOFF_CAN_ERROR,
    STUF_CAN_ERROR,
    FORM_CAN_ERROR,
    ACK_CAN_ERROR,
    BIT1_CAN_ERROR,
    BIT0_CAN_ERROR,
    CRC_CAN_ERROR,
    UNUSED_CAN_ERROR,
    UNRECOGNIZED_MSGOBJ_CAN_ERROR,
    UNRECOGNIZED_ERROR_CODE,
    TX_BUFFER_FULL_CAN_ERROR,
    RX_BUFFER_FULL_CAN_ERROR
} CAN_ERROR_T;

void CAN_Init(uint32_t baud_rate);
void CAN_SetMask1(uint32_t mask, uint32_t mode_id);
void CAN_SetMask2(uint32_t mask, uint32_t mode_id);
CAN_ERROR_T CAN_Receive(CCAN_MSG_OBJ_T* user_buffer);
CAN_ERROR_T CAN_Transmit(uint32_t msg_id, uint8_t* data, uint8_t data_len);
CAN_ERROR_T CAN_TransmitMsgObj(CCAN_MSG_OBJ_T *msg_obj);
CAN_ERROR_T CAN_GetErrorStatus(void);

uint8_t CAN_GetTxErrorCount(void);
uint8_t CAN_GetRxErrorCount(void);
bool CAN_IsTXBusy(void);
bool CAN_ResetPeripheral(void);

#endif
#endif
