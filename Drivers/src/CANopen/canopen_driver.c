/***********************************************************************
 * $Id:: canopen_driver.c 1604 2012-04-24 11:34:47Z nxp31103     $
 *
 * Project: CANopen Application Example for LPC11Cxx
 *
 * Description:
 *   CANopen driver source file
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

#include "canopen_driver.h"
#include "CAN_Node_Def.h"

#if defined (  __GNUC__  )						/* only for LPCXpresso, for keil/IAR this is done by linker */
#include "cr_section_macros.h"
__BSS(RESERVED) char CAN_driver_memory[184]; 	/* reserve 184 bytes for CAN driver */
#endif

/* CANopen configuration structure */
CAN_CANOPENCFG myCANopen =
{
	CAN_NODE_ID,							/* node_id */
	1,										/* msgobj_rx */
	2,										/* msgobj_tx */
	1,										/* isr_handled */
	(uint32_t)NULL,							/* od_const_num, set to right value in CANopen initialization */
	(CAN_ODCONSTENTRY *)myConstOD,			/* od_const_table */
	(uint32_t)NULL,							/* od_num, set to right value in CANopen initialization */
	(CAN_ODENTRY *)myOD,					/* od_table */
};

/* Publish CAN Callback Functions of onchip drivers */
CAN_CALLBACKS callbacks = {
	CAN_RX,										/* callback for any message received CAN frame which ID matches with any of the message objects' masks */
	CAN_TX,										/* callback for every transmitted CAN frame */
	CAN_Error,									/* callback for CAN errors */
	CANopen_SDOS_Exp_Read,						/* callback for expedited read access (SDO server) */
	CANopen_SDOS_Exp_Write,						/* callback for expedited write access (SDO server) */
	CANopen_SDOS_Seg_Read,						/* callback for segmented read access (SDO server) */
	CANopen_SDOS_Seg_Write,						/* callback for segmented write access (SDO server) */
	NULL,										/* callback for fall-back SDO handler (not used) */
};

/* Global variables */
ROM **rom = (ROM **)0x1FFF1FF8;					/* ROM entrypoint for on-chip CAN drivers */
volatile uint32_t CANopen_Timeout;				/* CANopen timeout timer for SDO client */
volatile uint8_t* CANopen_SDOC_Exp_ValidBytes;	/* Number of valid bytes for SDO client expedited read */
volatile uint32_t CANopen_SDOC_InBuff;			/* Number of bytes in SDO client buffer for segmented read/write */
volatile uint8_t* CANopen_SDOC_Buff;			/* Buffer for SDO client segmented read/write */
volatile uint32_t CANopen_SDOC_Seg_BuffSize;	/* Size of SDO client buffer in number of bytes */
volatile uint16_t CANopen_SDOC_Seg_ID;			/* Target node ID for SDO client segmented read/write */
volatile uint8_t CANopen_NMT_MyState;			/* Present state of NMT state machine */
volatile uint8_t CANopen_SDOC_State;			/* Present state of SDO client state machine for controlling SDO client transfers */
volatile uint32_t CANopen_Heartbeat_Producer_Counter;

/*****************************************************************************
** Function name:		CANopenInit
**
** Description:			Initializes CAN / CANopen
** 						Function should be executed before using the CAN bus.
** 						Initializes the CAN controller, on-chip drivers and
** 						SDO/NMT.
**
** Parameters:			None
** Returned value:		None
*****************************************************************************/
void CANopenInit(void)
{
	CAN_MSG_OBJ CANopen_Msg_Obj;
	uint32_t ClkInitTable[2] = {		/* Initialize CAN Controller structure*/
	  0x00000000UL, 					/* CANCLKDIV */
	  0x00001C57UL  					/* CAN_BTR */
	};

	/* Initialize the CAN controller */
	(*rom)->pCAND->init_can(&ClkInitTable[0], 1);

	/* Configure the CAN callback functions */
	(*rom)->pCAND->config_calb(&callbacks);

	/* Initialize CANopen handler */
	myCANopen.od_const_num = NumberOfmyConstODEntries;
	myCANopen.od_num = NumberOfmyODEntries;
	(*rom)->pCAND->config_canopen((CAN_CANOPENCFG *)&myCANopen);

	/* Enable the CAN Interrupt */
	NVIC_EnableIRQ(CAN_IRQn);

	/* Configure message object 3 to receive all 11-bit messages 0x700-0x77F */
	CANopen_Msg_Obj.msgobj = 3;
	CANopen_Msg_Obj.mode_id = 0x700;
	CANopen_Msg_Obj.mask = 0x780;
	(*rom)->pCAND->config_rxmsgobj(&CANopen_Msg_Obj);

	/* Configure message object 5 to receive all 11-bit messages 0x000 */
	CANopen_Msg_Obj.msgobj = 5;
	CANopen_Msg_Obj.mode_id = 0x000;
	CANopen_Msg_Obj.mask = 0x7FF;
	(*rom)->pCAND->config_rxmsgobj(&CANopen_Msg_Obj);

	/* Configure message object 7 to receive all 11-bit messages 0x580-5FF */
	CANopen_Msg_Obj.msgobj = 7;
	CANopen_Msg_Obj.mode_id = 0x580;
	CANopen_Msg_Obj.mask = 0x780;
	(*rom)->pCAND->config_rxmsgobj(&CANopen_Msg_Obj);
	CANopen_Init_SDO();

	/* change state and send bootup */
	CANopen_NMT_Change_MyState(NMT_CMD_ENTER_PRE_OP);
}

/*****************************************************************************
** Function name:		CANopen_1ms_tick
**
** Description:			CAN related functions executed every 1ms, e.g:
** 						SDO timeout, heartbeat consuming/generation.
** 						Function must be called from application every 1ms.
**
** Parameters:			None
** Returned value:		None
*****************************************************************************/
void CANopen_1ms_tick(void)
{
	uint32_t i;
	static uint8_t reInit_ID;			/* ID for restoring heartbeat producer / consumer after heartbeat failure */
	static uint8_t reInit_buf[2][4];	/* buffer for restoring heartbeat producer / consumer after heartbeat failure */
	static uint8_t reInit_state = 0;	/* simple statemachine for storing heartbeat producer / consumer after heartbeat failure */

	/* timeout for SDO client functions */
	if(!(CANopen_SDOC_State == CANopen_SDOC_Succes || CANopen_SDOC_State == CANopen_SDOC_Fail))
	{
		if(CANopen_Timeout-- == 0)
			CANopen_SDOC_State = CANopen_SDOC_Fail;		/* change status to fail on timeout */
	}

	/* if heartbeat producer value is specified, send heartbeat after expiring specified time */
	if(CANopen_Heartbeat_Producer_Value)
	{
		if((++(CANopen_Heartbeat_Producer_Counter)) >= CANopen_Heartbeat_Producer_Value)
		{
			CANopen_Heartbeat_Producer_Counter = 0;
			CANopen_Heartbeat_Send();
		}
	}

	/* if any watchnodes are specified, keep track of their heartbeat here */
	for(i=0; i<WatchListLength; i++)
	{
		if(WatchList[i].value & 0x0000FFFF)
		{
			/* specified node should produce heartbeat */
			if((++(WatchList[i].counter)) >= (WatchList[i].value & 0x0000FFFF))
			{
				/* counter is higher than value, so heartbeat is not received in time */
				WatchList[i].status = NMT_STATE_INTIALIZING;
				CANopen_NMT_Send_CMD((WatchList[i].value >> 16)&0x7F, NMT_CMD_RESET_COMM);
				CANopen_Heartbeat_Consumer_Failed((WatchList[i].value >> 16)&0x7F);
				WatchList[i].counter = 0;
				WatchList[i].heartbeatFail = 1;
			}
			if(WatchList[i].heartbeatFail && WatchList[i].BootupAfterHBF)
			{
				/* heartbeat has failed once, now a bootup message has been received from this node */
				WatchList[i].heartbeatFail = 0;
				WatchList[i].BootupAfterHBF = 0;
				if(WatchList[i].ProducerTime && WatchList[i].NodeID)
				{
					/* ProducerTime and NodeID specified, so reinitialize ProducerTime */
					reInit_ID = WatchList[i].NodeID;
					reInit_buf[0][0] = (WatchList[i].ProducerTime >> 8) & 0xFF;
					reInit_buf[0][1] = (WatchList[i].ProducerTime >> 0) & 0xFF;
					reInit_state |= 0x01;
				}
				if(WatchList[i].ConsumerTime && WatchList[i].NodeID)
				{
					/* ConsumerTime and NodeID specified, so reinitialize ConsumerTime */
					reInit_ID = WatchList[i].NodeID;
					reInit_buf[1][0] = 0x00;
					reInit_buf[1][1] = CAN_NODE_ID;
					reInit_buf[1][2] = (WatchList[i].ConsumerTime >> 8) & 0xFF;
					reInit_buf[1][3] = (WatchList[i].ConsumerTime >> 0) & 0xFF;
					reInit_state |= 0x04;
				}
			}
		}
	}

	/* if any of the nodes specified in the watchlist fails, re-initialize them after bootup */
	if(reInit_state)
	{
		if(reInit_state & 0x01)
		{
			/* send SDO expedited write message to re-init producer time */
			if(CANopen_SDOC_Exp_Write(reInit_ID, 0x1017, 0x00, (uint8_t*)reInit_buf[0], 2))
			{
				reInit_state &= ~0x01;
				reInit_state |= 0x02;
			}
		}
		else if(reInit_state & 0x02)
		{
			/* wait until SDO expedited write has been finished */
			if(CANopen_SDOC_State == CANopen_SDOC_Succes)
				reInit_state &= ~0x02;							/* succes, producer time restored */
			else if(CANopen_SDOC_State == CANopen_SDOC_Fail)
			{
				reInit_state &= ~0x02;							/* fail, try again */
				reInit_state |= 0x01;
			}
		}
		else if(reInit_state & 0x04)
		{
			/* send SDO expedited write message to re-init consumer time */
			if(CANopen_SDOC_Exp_Write(reInit_ID, 0x1016, 0x01, (uint8_t*)reInit_buf[1], 4))
			{
				reInit_state &= ~0x04;
				reInit_state |= 0x08;
			}
		}
		else if(reInit_state & 0x08)
		{
			/* wait until SDO expedited write has been finished */
			if(CANopen_SDOC_State == CANopen_SDOC_Succes)
				reInit_state &= ~0x08;							/* succes, consumer time restored */
			else if(CANopen_SDOC_State == CANopen_SDOC_Fail)
			{
				reInit_state &= ~0x08;							/* fail, try again */
				reInit_state |= 0x04;
			}
		}
	}
}

/*****************************************************************************
** Function name:		CAN_IRQHandler
**
** Description:			CAN interrupt handler.
** 						The CAN interrupt handler must be provided by the user application.
**						It's function is to call the isr() API located in the ROM
**
** Parameters:			None
** Returned value:		None
*****************************************************************************/
void CAN_IRQHandler (void)
{
  (*rom)->pCAND->isr();
}

/*****************************************************************************
** Function name:		CAN_RX
**
** Description:			CAN receive callback.
** 						Function is executed by the Callback handler after
**						a CAN message has been received
**
** Parameters:			msg_obj_num. Contains the number of the message object
** 						that triggered the CAN receive callback.
** Returned value:		None
*****************************************************************************/
void CAN_RX(uint8_t msg_obj_num)
{
	uint32_t i;
	CAN_MSG_OBJ CANopen_Msg_Obj;

	/* Determine which CAN message has been received */
	CANopen_Msg_Obj.msgobj = msg_obj_num;

	/* Now load up the CANopen_Msg_Obj structure with the CAN message */
	(*rom)->pCAND->can_receive(&CANopen_Msg_Obj);

	if(msg_obj_num == 3)
	{
		/* message object used for heartbeat / bootup */
		for(i=0; i<WatchListLength; i++)
		{
			if((CANopen_Msg_Obj.mode_id & 0x7F) == WatchList[i].NodeID || (CANopen_Msg_Obj.mode_id & 0x7F) == ((WatchList[i].value>>16) & 0x007F))
			{
				/* Node ID of received message is listed in watchlist */
				WatchList[i].counter = 0;
				WatchList[i].status = CANopen_Msg_Obj.data[0];
				if(CANopen_Msg_Obj.data[0] == 0x00)
				{
					/* received message is bootup */
					WatchList[i].status = NMT_STATE_PRE_OPERATIONAL;			/* Received bootup, thus state is pre-op */
					if(WatchList[i].heartbeatFail)
						WatchList[i].BootupAfterHBF = 1;
					CANopen_NMT_Consumer_Bootup_Received(CANopen_Msg_Obj.mode_id & 0x7F);
				}
			}
		}
	}

	if (msg_obj_num == 5)
	{
		/* message object used for NMT */
		if(CANopen_Msg_Obj.data[1] == CAN_NODE_ID || CANopen_Msg_Obj.data[1] == 0x00)
			CANopen_NMT_Change_MyState(CANopen_Msg_Obj.data[0]);			/* change NMT state both on broadcast and on my ID */
	}

	if (msg_obj_num == 7)
	{
		/* message object used for SDO client */
		if(CANopen_SDOC_State == CANopen_SDOC_Exp_Read_Busy)
		{
			/* Expedited read was initiated */
			if((CANopen_Msg_Obj.data[0] & (7<<5)) == 0x40)
			{
				/* received data from server */
				i = 4-((CANopen_Msg_Obj.data[0]>>2) & 0x03);				/* i now contains number of valid data bytes */
				CANopen_SDOC_InBuff = 0;

				while(i--)
					CANopen_SDOC_Buff[i] = CANopen_Msg_Obj.data[CANopen_SDOC_InBuff++ + 4];	/* save valid databytes to memory */
				CANopen_SDOC_State = CANopen_SDOC_Succes;					/* expedited read completed successfully */
				if(CANopen_SDOC_Exp_ValidBytes)
					*CANopen_SDOC_Exp_ValidBytes = CANopen_SDOC_InBuff;		/* save number of valid bytes */
			}
		}
		else if(CANopen_SDOC_State == CANopen_SDOC_Exp_Write_Busy)
		{
			/* expedited write was initiated */
			if(CANopen_Msg_Obj.data[0] == 0x60)
				CANopen_SDOC_State = CANopen_SDOC_Succes;					/* received confirmation */
		}
		else if(CANopen_SDOC_State == CANopen_SDOC_Seg_Read_Busy)
		{
			/* segmented read was initiated */
			if(((CANopen_Msg_Obj.data[0] & (7<<5)) == 0x40) && ((CANopen_Msg_Obj.data[0] & (1<<1)) == 0x00))
			{
				/* Received reply on initiate command, send first segment request */
				CANopen_Msg_Obj.msgobj = 8;
				CANopen_Msg_Obj.mode_id = 0x600 + CANopen_SDOC_Seg_ID;
				CANopen_Msg_Obj.data[0] = 0x60;
				CANopen_Msg_Obj.data[1] = 0x00;
				CANopen_Msg_Obj.data[2] = 0x00;
				CANopen_Msg_Obj.data[3] = 0x00;
				CANopen_Msg_Obj.data[4] = 0x00;
				CANopen_Msg_Obj.data[5] = 0x00;
				CANopen_Msg_Obj.data[6] = 0x00;
				CANopen_Msg_Obj.data[7] = 0x00;
				(*rom)->pCAND->can_transmit(&CANopen_Msg_Obj);

				CANopen_SDOC_InBuff = 0;
			}
			else if((CANopen_Msg_Obj.data[0] & (7<<5)) == 0x00)
			{
				/* Received response on request */
				for(i=0; i < (7 - ((CANopen_Msg_Obj.data[0]>>1) & 0x07)); i++)
				{
					/* get all data from frame and save it to memory */
					if(CANopen_SDOC_InBuff < CANopen_SDOC_Seg_BuffSize)
						CANopen_SDOC_Buff[CANopen_SDOC_InBuff++] = CANopen_Msg_Obj.data[i+1];
					else
					{
						/* SDO segment too big for buffer, abort */
						CANopen_SDOC_State = CANopen_SDOC_Fail;
					}
				}

				if(CANopen_Msg_Obj.data[0] & 0x01)
				{
					/* Last frame, change status to success */
					CANopen_SDOC_State = CANopen_SDOC_Succes;
				}
				else
				{
					/* not last frame, send acknowledge */
					CANopen_Msg_Obj.msgobj = 8;
					CANopen_Msg_Obj.mode_id = 0x600 + CANopen_SDOC_Seg_ID;
					CANopen_Msg_Obj.data[0] = 0x60 | ((CANopen_Msg_Obj.data[0] & (1<<4)) ^ (1<<4));		/* toggle */
					CANopen_Msg_Obj.data[1] = 0x00;
					CANopen_Msg_Obj.data[2] = 0x00;
					CANopen_Msg_Obj.data[3] = 0x00;
					CANopen_Msg_Obj.data[4] = 0x00;
					CANopen_Msg_Obj.data[5] = 0x00;
					CANopen_Msg_Obj.data[6] = 0x00;
					CANopen_Msg_Obj.data[7] = 0x00;
					(*rom)->pCAND->can_transmit(&CANopen_Msg_Obj);
				}
			}
		}
		else if(CANopen_SDOC_State == CANopen_SDOC_Seg_Write_Busy)
		{
			/* segmented write was initiated */
			if((((CANopen_Msg_Obj.data[0] & (7<<5)) == 0x60) && ((CANopen_Msg_Obj.data[0] & (1<<1)) == 0x00)) || ((CANopen_Msg_Obj.data[0] & (7<<5)) == 0x20))
			{
				/* received acknowledge */
				CANopen_Msg_Obj.msgobj = 8;
				CANopen_Msg_Obj.mode_id = 0x600 + CANopen_SDOC_Seg_ID;
				if((CANopen_Msg_Obj.data[0] & (7<<5)) == 0x60)
				{
					/* first frame */
					CANopen_SDOC_InBuff = 0;			/* Clear buffer */
					CANopen_Msg_Obj.data[0] = 1<<4;		/* initialize for toggle */
				}
				CANopen_Msg_Obj.data[0] = ((CANopen_Msg_Obj.data[0] & (1<<4)) ^ (1<<4));		/* toggle */

				/* fill frame data */
				for(i=0; i<7; i++)
				{
					if(CANopen_SDOC_InBuff < CANopen_SDOC_Seg_BuffSize)
						CANopen_Msg_Obj.data[i+1] = CANopen_SDOC_Buff[CANopen_SDOC_InBuff++];
					else
						CANopen_Msg_Obj.data[i+1] = 0x00;
				}

				/* if end of buffer has been reached, then this is the last frame */
				if(CANopen_SDOC_InBuff == CANopen_SDOC_Seg_BuffSize)
				{
					CANopen_Msg_Obj.data[0] |= ((7-(CANopen_SDOC_Seg_BuffSize%7))<<1) | 0x01;		/* save length */
					CANopen_SDOC_State = CANopen_SDOC_Succes;										/* set state to succes */
				}

				(*rom)->pCAND->can_transmit(&CANopen_Msg_Obj);
			}
		}
	}
	return;
}

/*****************************************************************************
** Function name:		CAN_TX
**
** Description:			CAN transmit callback.
** 						Function is executed by the Callback handler after
**						a CAN message has been transmitted
**
** Parameters:			msg_obj_num. Contains the number of the message object
** 						that triggered the CAN transmit callback.
** Returned value:		None
*****************************************************************************/
void CAN_TX(uint8_t msg_obj_num){
  return;
}

/*****************************************************************************
** Function name:		CAN_Error
**
** Description:			CAN error callback.
** 						Function is executed by the Callback handler after
**						an error has occured on the CAN bus
**
** Parameters:			error_info. Contains the error code
** 						that triggered the CAN error callback.
** Returned value:		None
*****************************************************************************/
void CAN_Error(uint32_t error_info){
  return;
}

/*****************************************************************************
** Function name:		CANopen_NMT_Send_CMD
**
** Description:			Send NMT Command.
** 						Call function to send NMT command.
**
** Parameters:			ID. Node ID to target NMT command
** 						CMD. NMT command to send to the node ID
** Returned value:		None
*****************************************************************************/
void CANopen_NMT_Send_CMD(uint8_t ID, uint8_t CMD)
{
	static CAN_MSG_OBJ msg_NMT_TX =
	{
			0x00,						/* mode_id */
			0x00,						/* mask */
			{
					0x00,				/* data[0] */
					0x00,				/* data[1] */
					0x00,				/* data[2] */
					0x00,				/* data[3] */
					0x00,				/* data[4] */
					0x00,				/* data[5] */
					0x00,				/* data[6] */
					0x00,				/* data[7] */
			},
			2,							/* dlc */
			6							/* msgobj */
	};
	msg_NMT_TX.data[0] = CMD;
	msg_NMT_TX.data[1] = ID & 0x7F;
	(*rom)->pCAND->can_transmit(&msg_NMT_TX);
}

/*****************************************************************************
** Function name:		CANopen_NMT_Change_MyState
**
** Description:			Change NMT state-machine.
** 						Call to change the state of the NMT state machine
**
** Parameters:			NMT_Command. Determines the NMT state.
** Returned value:		None
*****************************************************************************/
void CANopen_NMT_Change_MyState(uint8_t NMT_Command)
{
	switch(NMT_Command)
	{
		case NMT_CMD_ENTER_PRE_OP:
			/* received enter pre-operational mode */
			if(CANopen_NMT_MyState == NMT_STATE_INTIALIZING || CANopen_NMT_MyState == NMT_STATE_OPERATIONAL || CANopen_NMT_MyState == NMT_STATE_STOPPED)
			{
				/* only change to this new state if this is allowed from previous state */
				if(CANopen_NMT_MyState == NMT_STATE_INTIALIZING)		/* if previous state was initializing */
					CANopen_Heartbeat_Send();							/* then send bootup message */
				CANopen_NMT_MyState = NMT_STATE_PRE_OPERATIONAL;		/* Change state to Pre-Operational */
			}
			break;
		case NMT_CMD_START:
			/* received enter operational mode */
			if(CANopen_NMT_MyState == NMT_STATE_PRE_OPERATIONAL || CANopen_NMT_MyState == NMT_STATE_STOPPED)
			{
				/* only change to this new state if this is allowed from previous state */
				CANopen_NMT_MyState = NMT_STATE_OPERATIONAL;
			}
			break;
		case NMT_CMD_STOP:
			/* received enter stopped mode */
			if(CANopen_NMT_MyState == NMT_STATE_PRE_OPERATIONAL || CANopen_NMT_MyState == NMT_STATE_OPERATIONAL)
			{
				/* only change to this new state if this is allowed from previous state */
				CANopen_NMT_MyState = NMT_STATE_STOPPED;
			}
			break;
		case NMT_CMD_RESET_NODE:
			/* received enter reset node mode, allowed from all states */
			CANopen_NMT_MyState = NMT_STATE_INTIALIZING;
			CANopen_NMT_Reset_Node_Received();
			CANopen_Init_SDO();
			CANopen_NMT_Change_MyState(NMT_CMD_ENTER_PRE_OP);
			break;
		case NMT_CMD_RESET_COMM:
			/* received enter reset communication mode, allowed from all states */
			CANopen_NMT_MyState = NMT_STATE_INTIALIZING;
			CANopen_NMT_Reset_Comm_Received();
			CANopen_Init_SDO();
			CANopen_NMT_Change_MyState(NMT_CMD_ENTER_PRE_OP);
			break;
	}
}

/*****************************************************************************
** Function name:		CANopen_Heartbeat_Send
**
** Description:			Send heartbeat message.
** 						Call function to send heartbeat message.
**
** Parameters:			None
** Returned value:		None
*****************************************************************************/
void CANopen_Heartbeat_Send(void)
{
	static CAN_MSG_OBJ msg_HeartbeatTX =
	{
			0x700 + CAN_NODE_ID,		/* mode_id */
			0x00,						/* mask */
			{
					0x00,				/* data[0] */
					0x00,				/* data[1] */
					0x00,				/* data[2] */
					0x00,				/* data[3] */
					0x00,				/* data[4] */
					0x00,				/* data[5] */
					0x00,				/* data[6] */
					0x00,				/* data[7] */
			},
			1,							/* dlc */
			4							/* msgobj */
	};

	msg_HeartbeatTX.data[0] = CANopen_NMT_MyState;
	(*rom)->pCAND->can_transmit(&msg_HeartbeatTX);
}

/*****************************************************************************
** Function name:		CANopen_SDOS_Exp_Read
**
** Description:			CANopen Callback for expedited read accesses
**
** Parameters:			index: Index used in expedited read access that triggered
** 						the CANopen expedited read access Callback
** 						subindex: Sub index used in expedited read access that triggered
** 						the CANopen expedited read access Callback
** Returned value:		Return 0 for successs, SDO Abort code for error
*****************************************************************************/
uint32_t CANopen_SDOS_Exp_Read(uint16_t index, uint8_t subindex)
{
	return 0;  /* Return 0 for successs, SDO Abort code for error */
}

/*****************************************************************************
** Function name:		CANopen_SDOS_Exp_Write
**
** Description:			CANopen Callback for expedited write accesses
**
** Parameters:			index: Index used in expedited write access that triggered
** 						the CANopen expedited write access Callback
** 						subindex: Sub index used in expedited write access that triggered
** 						the CANopen expedited write access Callback
** 						dat_ptr: pointer to value_pointer as specified in OD at the
** 						specific index/subindex
** Returned value:		Return 0 for successs, SDO Abort code for error
*****************************************************************************/
uint32_t CANopen_SDOS_Exp_Write(uint16_t index, uint8_t subindex, uint8_t *dat_ptr)
{
	return 0;  /* Return 0 for successs, SDO Abort code for error */
}

/*****************************************************************************
** Function name:		CANopen_SDOS_Seg_Read
**
** Description:			CANopen Callback for segmented read accesses.
**
** Parameters:			index: Index used in segmented read access that triggered
** 						the CANopen segmented read access Callback
** 						subindex: Sub index used in segmented read access that triggered
** 						the CANopen segmented read access Callback
** 						openclose: Begin/mid/end of reading segment. Possible values:
** 						CAN_SDOSEG_OPEN, CAN_SDOSEG_SEGMENT, CAN_SDOSEG_CLOSE
** 						length: Number of bytes available in data to be
** 						transfered to client
** 						data: buffer for data to be transfered to client
** 						last: if true, this CAN frame holds the last data-bytes of
** 						the segmented read access
** Returned value:		Return 0 for successs, SDO Abort code for error
*****************************************************************************/
uint32_t CANopen_SDOS_Seg_Read(uint16_t index, uint8_t subindex, uint8_t openclose, uint8_t *length, uint8_t *data, uint8_t *last)
{
	static uint16_t read_ofs;					/* read index for data */
	static SDOS_Buffer_t* SDOS_Buffer = NULL;	/* pointer to SDOS buffer */
	uint8_t access_type;						/* access type (seg/exp, RO, WO, RW, etc) */
	uint16_t i;

	if(openclose == CAN_SDOSEG_OPEN)
	{
		/* new connection opened */
		CANopen_SDOS_Find_Buffer(index, subindex, &SDOS_Buffer, &access_type);		/* search appropriate buffer */
		if(!SDOS_Buffer)
			return SDO_ABORT_NOT_EXISTS;											/* no buffer found */
		if(access_type != OD_SEG_RO && access_type != OD_SEG_RW)
		{
			/* No segment read is allowed */
			SDOS_Buffer = NULL;
			return SDO_ABORT_WRITEONLY;
		}
		read_ofs = 0;
	}
	if(SDOS_Buffer)
	{
		/* buffer with right access type has been found */
		if(openclose == CAN_SDOSEG_SEGMENT)
		{
			/* transfer in progress */
			i = 7;
			/* copy data from memory to frame */
			while (i && (read_ofs < SDOS_Buffer->length))
			{
				*data++ = (SDOS_Buffer->data)[read_ofs++];
				i--;
			}

			*length = 7-i;						/* save length */

			while(i--)
				*data++ = 0x00;					/* fill unused data frame with 0x00 */

			if (read_ofs == SDOS_Buffer->length)
			{
				/* The whole buffer read: this is last segment */
				*last = TRUE;
				SDOS_Buffer = NULL;
			}
		}
		if(openclose == CAN_SDOSEG_CLOSE)
			SDOS_Buffer = NULL;					/* closing connection */
		return 0;
	}
	else
		return SDO_ABORT_NOT_EXISTS;
}

/*****************************************************************************
** Function name:		CANopen_SDOS_Seg_Write
**
** Description:			CANopen callback for segmented write accesses.
**
** Parameters:			index: Index used in segmented write access that triggered
** 						the CANopen segmented write access Callback
** 						subindex: Sub index used in segmented write access that triggered
** 						the CANopen segmented write access Callback
** 						openclose: Begin/mid/end of writing segment. Possible values:
** 						CAN_SDOSEG_OPEN, CAN_SDOSEG_SEGMENT, CAN_SDOSEG_CLOSE
** 						length: Number of bytes available in data
** 						data: buffer for data from client to write to SDO entry
** 						fast_resp: if true, response will be only 1 byte (faster,
** 						but not supported by all SDO clients). When false, response
** 						will be 8 byte (CANopen compliant)
** Returned value:		Return 0 for successs, SDO Abort code for error
*****************************************************************************/
uint32_t CANopen_SDOS_Seg_Write(uint16_t index, uint8_t subindex, uint8_t openclose, uint8_t length, uint8_t *data, uint8_t *fast_resp)
{
	static uint16_t write_ofs;					/* write index for data */
	static SDOS_Buffer_t* SDOS_Buffer = NULL;	/* pointer to SDOS buffer */
	uint8_t access_type;						/* access type (seg/exp, RO, WO, RW, etc) */
	uint16_t i;

	if(openclose == CAN_SDOSEG_OPEN)
	{
		/* new connection opened */
		CANopen_SDOS_Find_Buffer(index, subindex, &SDOS_Buffer, &access_type);	/* search appropriate buffer */
		if(!SDOS_Buffer)
			return SDO_ABORT_NOT_EXISTS;											/* no buffer found */
		if(access_type != OD_SEG_WO && access_type != OD_SEG_RW)
		{
			/* No segment read is allowed */
			SDOS_Buffer = NULL;		/* No segment read is allowed */
			return SDO_ABORT_READONLY;
		}
		write_ofs = 0;
	}
	if(SDOS_Buffer)
	{
		/* buffer with right access type has been found */
		if(openclose == CAN_SDOSEG_SEGMENT)
		{
			/* transfer in progress */
			*fast_resp = FALSE; 						/* Do not use fast 1-byte segment write response */
			i = length;

			/* copy data from frame to memory */
			while (i && (write_ofs < SDOS_Buffer->length))
			{
				(SDOS_Buffer->data)[write_ofs++] = *data++;
				i--;
			}
			if (i && (write_ofs >= SDOS_Buffer->length))
				return SDO_ABORT_TRANSFER; 					/* Too much data to write */
		}
		else if (openclose == CAN_SDOSEG_CLOSE)
		{
			/* Write has successfully finished */
			SDOS_Buffer = NULL;
		}
		return 0;
	}
	else
		return SDO_ABORT_NOT_EXISTS;
}

/*****************************************************************************
** Function name:		CANopen_SDOS_Find_Buffer
**
** Description:			Searches OD to find buffer matching with a specific
** 						index / sub index for segmented access.
**
** Parameters:			index: Index used in segmented access
** 						subindex: Sub index used in segmented access
** 						SDOS_Buffer: pointer to pointer to buffer to SDOS buffer
** 						access_type: pointer to variable indicating type of entry
** Returned value:		none
*****************************************************************************/
void CANopen_SDOS_Find_Buffer(uint16_t index, uint8_t subindex, SDOS_Buffer_t** SDOS_Buffer, uint8_t* access_type)
{
	uint32_t i;

	*SDOS_Buffer = NULL;
	for(i=0; i< NumberOfmyODEntries; i++)
	{
		/* scan all items in OD for the specified index/subindex  */
		if(myOD[i].index == index && myOD[i].subindex == subindex)
		{
			//OD entry exists, now load pointers
			*SDOS_Buffer = (SDOS_Buffer_t*)myOD[i].val;
			*access_type = myOD[i].entrytype_len;
			break;
		}
	}
}

/*****************************************************************************
** Function name:		CANopen_SDOC_Exp_Read
**
** Description:			Sends request to read expedited data from server.
**
** Parameters:			node_id: Node ID of server to reach
** 						index: Index of OD of server to read
** 						subindex: Index of OD of server to read
** 						data: pointer to buffer to store the read data
** 						valid_data_bytes: pointer to variable to store number of
** 						valid bytes stored in data
** Returned value:		Returns 1 for success, 0 for error
*****************************************************************************/
uint8_t CANopen_SDOC_Exp_Read(uint8_t node_id, uint16_t index, uint8_t subindex, uint8_t* data, uint8_t* valid_data_bytes)
{
	CAN_MSG_OBJ msg_SDO_Seg_TX;

	if(CANopen_SDOC_State == CANopen_SDOC_Succes || CANopen_SDOC_State == CANopen_SDOC_Fail)
	{
		/* only initiate when no SDO client transfer is ongoing */
		CANopen_SDOC_State = CANopen_SDOC_Exp_Read_Busy;
		CANopen_SDOC_InBuff = 0;
		CANopen_SDOC_Buff = data;
		CANopen_SDOC_Exp_ValidBytes = valid_data_bytes;
		msg_SDO_Seg_TX.mode_id = 0x600 + node_id;
		msg_SDO_Seg_TX.mask = 0x00;
		msg_SDO_Seg_TX.dlc = 8;
		msg_SDO_Seg_TX.msgobj = 8;
		msg_SDO_Seg_TX.data[0] = 0x02<<5;
		msg_SDO_Seg_TX.data[1] = index & 0xFF;
		msg_SDO_Seg_TX.data[2] = (index >> 8) & 0xFF;
		msg_SDO_Seg_TX.data[3] = subindex;
		msg_SDO_Seg_TX.data[4] = 0x00;
		msg_SDO_Seg_TX.data[5] = 0x00;
		msg_SDO_Seg_TX.data[6] = 0x00;
		msg_SDO_Seg_TX.data[7] = 0x00;

		(*rom)->pCAND->can_transmit(&msg_SDO_Seg_TX);
		CANopen_Timeout = CANOPEN_TIMEOUT_VAL;
		return 1;
	}
	CANopen_SDOC_State = CANopen_SDOC_Fail;
	return 0;
}

/*****************************************************************************
** Function name:		CANopen_SDOC_Exp_Write
**
** Description:			Sends request to write expedited data to server.
**
** Parameters:			node_id: Node ID of server to reach
** 						index: Index of OD of server to write
** 						subindex: sub index of OD of server to write
** 						data: pointer to buffer to data to write
** 						length: variable indicating the number of valid bytes
** 						in data
** Returned value:		Returns 1 for success, 0 for error
*****************************************************************************/
uint8_t CANopen_SDOC_Exp_Write(uint8_t node_id, uint16_t index, uint8_t subindex, uint8_t* data, uint8_t length)
{
	CAN_MSG_OBJ msg_SDO_Seg_TX;
	uint8_t i, i2;

	if(CANopen_SDOC_State == CANopen_SDOC_Succes || CANopen_SDOC_State == CANopen_SDOC_Fail)
	{
		CANopen_SDOC_State = CANopen_SDOC_Exp_Write_Busy;
		if(length == 0 || length > 4)
		{
			CANopen_SDOC_State = CANopen_SDOC_Fail;		/* min for exp write id 0, max is 4 */
			return 0;
		}

		msg_SDO_Seg_TX.mode_id = 0x600 + node_id;
		msg_SDO_Seg_TX.mask = 0x00;
		msg_SDO_Seg_TX.dlc = 8;
		msg_SDO_Seg_TX.msgobj = 8;
		msg_SDO_Seg_TX.data[0] = 1<<5 | (4-length)<<2 | 0x02 | 0x01;
		msg_SDO_Seg_TX.data[1] = index & 0xFF;
		msg_SDO_Seg_TX.data[2] = (index >> 8) & 0xFF;
		msg_SDO_Seg_TX.data[3] = subindex;
		msg_SDO_Seg_TX.data[4] = 0x00;
		msg_SDO_Seg_TX.data[5] = 0x00;
		msg_SDO_Seg_TX.data[6] = 0x00;
		msg_SDO_Seg_TX.data[7] = 0x00;

		i2 = 0;
		for(i=length; i!=0; i--)
			msg_SDO_Seg_TX.data[i+3] = data[i2++];

		(*rom)->pCAND->can_transmit(&msg_SDO_Seg_TX);
		CANopen_Timeout = CANOPEN_TIMEOUT_VAL;
		return 1;
	}
	CANopen_SDOC_State = CANopen_SDOC_Fail;
	return 0;
}

/*****************************************************************************
** Function name:		CANopen_SDOC_Seg_Read
**
** Description:			Sends request to read segmented data from server.
**
** Parameters:			node_id: Node ID of server to reach
** 						index: Index of OD of server to read
** 						subindex: sub index of OD of server to read
** 						buff: pointer to buffer to store the read data
** 						buffSize: variable indicating size of buff
** Returned value:		Returns 1 for success, 0 for error
*****************************************************************************/
uint8_t CANopen_SDOC_Seg_Read(uint8_t node_id, uint16_t index, uint8_t subindex, void* buff, uint32_t buffSize)
{
	CAN_MSG_OBJ msg_SDO_Seg_TX;

	if(CANopen_SDOC_State == CANopen_SDOC_Succes || CANopen_SDOC_State == CANopen_SDOC_Fail)
	{
		CANopen_SDOC_State = CANopen_SDOC_Seg_Read_Busy;
		CANopen_SDOC_Buff = buff;
		CANopen_SDOC_Seg_BuffSize = buffSize;
		CANopen_SDOC_Seg_ID = node_id;

		msg_SDO_Seg_TX.mode_id = 0x600 + CANopen_SDOC_Seg_ID;
		msg_SDO_Seg_TX.mask = 0x00;
		msg_SDO_Seg_TX.dlc = 8;
		msg_SDO_Seg_TX.msgobj = 8;
		msg_SDO_Seg_TX.data[0] = 0x40;
		msg_SDO_Seg_TX.data[1] = index & 0xFF;
		msg_SDO_Seg_TX.data[2] = (index >> 8) & 0xFF;
		msg_SDO_Seg_TX.data[3] = subindex;
		msg_SDO_Seg_TX.data[4] = 0x00;
		msg_SDO_Seg_TX.data[5] = 0x00;
		msg_SDO_Seg_TX.data[6] = 0x00;
		msg_SDO_Seg_TX.data[7] = 0x00;

		(*rom)->pCAND->can_transmit(&msg_SDO_Seg_TX);
		CANopen_Timeout = CANOPEN_TIMEOUT_VAL;
		return 1;
	}
	CANopen_SDOC_State = CANopen_SDOC_Fail;
	return 0;
}

/*****************************************************************************
** Function name:		CANopen_SDOC_Seg_Write
**
** Description:			Sends request to write segmented data to server.
**
** Parameters:			node_id: Node ID of server to reach
** 						index: Index of OD of server to write
** 						subindex: sub index of OD of server to write
** 						buff: pointer to buffer containing data to store
** 						length: number of bytes to write
** Returned value:		Returns 1 for success, 0 for error
*****************************************************************************/
uint8_t CANopen_SDOC_Seg_Write(uint8_t node_id, uint16_t index, uint8_t subindex, void* buff, uint32_t length)
{
	CAN_MSG_OBJ msg_SDO_Seg_TX;

	if(CANopen_SDOC_State == CANopen_SDOC_Succes || CANopen_SDOC_State == CANopen_SDOC_Fail)
	{
		CANopen_SDOC_State = CANopen_SDOC_Seg_Write_Busy;
		CANopen_SDOC_Buff = buff;
		CANopen_SDOC_Seg_BuffSize = length;
		CANopen_SDOC_Seg_ID = node_id;

		msg_SDO_Seg_TX.mode_id = 0x600 + CANopen_SDOC_Seg_ID;
		msg_SDO_Seg_TX.mask = 0x00;
		msg_SDO_Seg_TX.dlc = 8;
		msg_SDO_Seg_TX.msgobj = 8;
		msg_SDO_Seg_TX.data[0] = 0x21;
		msg_SDO_Seg_TX.data[1] = index & 0xFF;
		msg_SDO_Seg_TX.data[2] = (index >> 8) & 0xFF;
		msg_SDO_Seg_TX.data[3] = subindex;
		msg_SDO_Seg_TX.data[4] = CANopen_SDOC_Seg_BuffSize & 0xFF;
		msg_SDO_Seg_TX.data[5] = (CANopen_SDOC_Seg_BuffSize >> 8) & 0xFF;
		msg_SDO_Seg_TX.data[6] = (CANopen_SDOC_Seg_BuffSize >> 16) & 0xFF;
		msg_SDO_Seg_TX.data[7] = (CANopen_SDOC_Seg_BuffSize >> 24) & 0xFF;

		(*rom)->pCAND->can_transmit(&msg_SDO_Seg_TX);
		CANopen_Timeout = CANOPEN_TIMEOUT_VAL;
		return 1;
	}
	CANopen_SDOC_State = CANopen_SDOC_Fail;
	return 0;
}
