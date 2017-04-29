#ifndef _LEGACY_
#ifndef _CAN_H_
#define _CAN_H_

// integrates with and based on ccand_11x.h error types
// explanations of page 289 of lpc11cx4 user manual
typedef enum CAN_ERROR {
    NO_CAN_ERROR = 0,
    NO_RX_CAN_MESSAGE = 1,
    EPASS_CAN_ERROR = 2,
    WARN_CAN_ERROR_WARN = 3,
    BOFF_CAN_ERROR = 4,
    STUF_CAN_ERROR = 5,
    FORM_CAN_ERROR = 6,
    ACK_CAN_ERROR = 7,
    BIT1_CAN_ERROR = 8,
    BIT0_CAN_ERROR = 9,
    CRC_CAN_ERROR = 10,
    UNUSED_CAN_ERROR = 11,
    UNRECOGNIZED_MSGOBJ_CAN_ERROR = 12,
    UNRECOGNIZED_ERROR_CODE = 13,
    TX_BUFFER_FULL_CAN_ERROR = 14,
    RX_BUFFER_FULL_CAN_ERROR = 15
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
void CAN_ResetPeripheral(void);
void CAN_PrintStatus(void);

#endif
#endif
