/***********************************************************************
 * $Id:: canopen_driver.h 1604 2012-04-24 11:34:47Z nxp31103     $
 *
 * Project: CANopen Application Example for LPC11Cxx
 *
 * Description:
 *   CANopen driver header file
 *
 * Copyright(C) 2012, NXP Semiconductor
 * All rights reserved.
 *
 ***********************************************************************
 * Software that is described herein is for illustrative purposes only
 * which provides customers with programming information regarding the
 * products. This software is supplied "AS IS" without any warranties.
 * NXP Semiconductors assumes no responsibility or liability for the
 * use of the software, conveys no license or title under any patent,
 * copyright, or mask work right to the product. NXP Semiconductors
 * reserves the right to make changes in the software without
 * notification. NXP Semiconductors also make no representation or
 * warranty that such application will be suitable for the specified
 * use without further testing or modification.
 **********************************************************************/

#ifndef CANOPEN_DRIVER_H_
#define CANOPEN_DRIVER_H_

#include "rom_drivers.h"

/* Message buffers used by CANopen library:
CANopen driver RX		1
CANopen driver TX		2
heartbeat RX			3
heartbeat TX			4
NMT RX					5
NMT TX					6
SDO Client RX			7
SDO Client TX			8
*/

#ifndef NULL
#define NULL    ((void *)0)
#endif

#ifndef FALSE
#define FALSE   (0)
#endif

#ifndef TRUE
#define TRUE    (1)
#endif

/* NMT module control command */
#define NMT_CMD_START				1
#define NMT_CMD_STOP				2
#define NMT_CMD_ENTER_PRE_OP		128
#define NMT_CMD_RESET_NODE			129
#define NMT_CMD_RESET_COMM			130

/* Node State */
#define NMT_STATE_INTIALIZING		0
#define NMT_STATE_STOPPED			4
#define NMT_STATE_OPERATIONAL		5
#define NMT_STATE_PRE_OPERATIONAL	127

/* SDOC State */
#define CANopen_SDOC_Fail			0
#define CANopen_SDOC_Succes			1
#define CANopen_SDOC_Exp_Read_Busy	2
#define CANopen_SDOC_Exp_Write_Busy	3
#define CANopen_SDOC_Seg_Read_Busy	4
#define CANopen_SDOC_Seg_Write_Busy	5

typedef struct _SDOS_Buffer_t {
	uint8_t* data;					/* pointer to buffer */
	uint32_t length;				/* length in buffer */
}SDOS_Buffer_t;

typedef struct _WatchNode_t {
	uint32_t value;					/* heartbeat consumer value, accessible via SDO */
	uint32_t counter;				/* used for keeping track of heartbeat */
	uint8_t status;					/* status of node to watch, e.g. NMT_STATE_INTIALIZING, NMT_STATE_STOPPED, etc. */
	uint8_t heartbeatFail;			/* set when heartbeat was not received in time */
	uint8_t BootupAfterHBF;			/* set when bootup message has been received after a heartbeat failure */
	uint8_t NodeID;					/* when != NULL, the software will automatically reconfigure the node specified by NodeID after a heartbeat failure of the node NodeID  */
	uint16_t ProducerTime;			/* when != NULL together with NodeID != NULL, heartbeat producer time will be automatically restored after heartbeat failure of node NodeID */
	uint16_t ConsumerTime;			/* when != NULL together with NodeID != NULL, heartbeat consumer time will be automatically restored after heartbeat failure of node NodeID */
}WatchNode_t;

/* General CANopen functions */
void CANopenInit(void);
void CANopen_1ms_tick(void);
extern void CANopen_Init_SDO(void);
/* General CAN functions */
void CAN_IRQHandler (void);
void CAN_RX(uint8_t msg_obj_num);
void CAN_TX(uint8_t msg_obj_num);
void CAN_Error(uint32_t error_info);
/* CANopen NMT / Heartbeat functions */
void CANopen_NMT_Send_CMD(uint8_t ID, uint8_t CMD);
void CANopen_NMT_Change_MyState(uint8_t NMT_Command);
void CANopen_Heartbeat_Send(void);
extern void CANopen_NMT_Reset_Node_Received(void);
extern void CANopen_NMT_Reset_Comm_Received(void);
extern void CANopen_NMT_Consumer_Bootup_Received(uint8_t Node_ID);
extern void CANopen_Heartbeat_Consumer_Failed(uint8_t Node_ID);
/* CANopen SDO server-side functions. Read/write functions are called by callback mechanism */
uint32_t CANopen_SDOS_Exp_Read(uint16_t index, uint8_t subindex);
uint32_t CANopen_SDOS_Exp_Write(uint16_t index, uint8_t subindex, uint8_t *dat_ptr);
uint32_t CANopen_SDOS_Seg_Read(uint16_t index, uint8_t subindex, uint8_t openclose, uint8_t *length, uint8_t *data, uint8_t *last);
uint32_t CANopen_SDOS_Seg_Write(uint16_t index, uint8_t subindex, uint8_t openclose, uint8_t length, uint8_t *data, uint8_t *fast_resp);
void CANopen_SDOS_Find_Buffer(uint16_t index, uint8_t subindex, SDOS_Buffer_t** SDOS_Buffer, uint8_t* access_type);
/* CANopen SDO client-side functions */
uint8_t CANopen_SDOC_Exp_Read(uint8_t node_id, uint16_t index, uint8_t subindex, uint8_t* data, uint8_t* valid_data_bytes);
uint8_t CANopen_SDOC_Exp_Write(uint8_t node_id, uint16_t index, uint8_t subindex, uint8_t* data, uint8_t length);
uint8_t CANopen_SDOC_Seg_Read(uint8_t node_id, uint16_t index, uint8_t subindex, void* buff, uint32_t buffSize);
uint8_t CANopen_SDOC_Seg_Write(uint8_t node_id, uint16_t index, uint8_t subindex, void* buff, uint32_t Length);

/* Global Variables */
extern volatile uint8_t CANopen_NMT_MyState;		/* Present state of NMT state machine */
extern volatile uint8_t CANopen_SDOC_State;			/* Present state of SDO client state machine for controlling SDO client transfers */
extern volatile uint32_t CANopen_Heartbeat_Producer_Counter;	/* heartbeat producer counter, used for generating heartbeat */
#endif /* CANOPEN_DRIVER_H_ */
