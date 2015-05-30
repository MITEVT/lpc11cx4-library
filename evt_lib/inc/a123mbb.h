
#ifndef _A123DFS_H_
#define _A123DFS_H_


#include "lpc_types.h"
#include "ccand_11xx.h"

//-------------------------
// Type Converters

#define num2mVolts(num) (uint32_t)(num/2+1000)
#define mVolts2Num(mvolts) (uint32_t)((mvolts-1000)*2)

#define num2degC(num) (int8_t)((num/2)-40)

//-------------------------
// BCM_CMD

#define BCM_CMD             0x50
#define BCM_CMD_DLC 		 3
#define BCM_REQUEST_TYPE_STD 0
#define BCM_REQUEST_TYPE_EXT 1
#define BCM_BALANCE_OFF num2mVolts(0x1FFE)

typedef struct {
	uint8_t request_type;
	uint8_t request_id;
	uint16_t balance_target_mVolts;
} MBB_CMD_T;

void MBB_MakeCMD(MBB_CMD_T *contents, CCAN_MSG_OBJ_T *msg_obj);

//-------------------------
// MBB_STD

#define MBB_STD             0x200
#define MBB_STD_MASK 		0xF00
#define MBB_STD_DLC 		8

typedef struct {
	uint8_t cell_overvolt;
	uint8_t cell_undervolt;
	uint8_t response_id;
	int8_t temp_degC;
	uint32_t mod_min_mVolts;
	uint8_t temp_chn;
	uint32_t mod_max_mVolts;
	uint8_t balance_c_count;
	uint32_t mod_avg_mVolts;
	uint8_t voltage_mismatch;
} MBB_STD_T;

int MBB_DecodeStd(MBB_STD_T *contents, CCAN_MSG_OBJ_T *msg_obj);


/*
** Data Offsets
*/
#define MOD_THERM_OFFSET -40
#define MOD_V_OFFSET 1

#endif