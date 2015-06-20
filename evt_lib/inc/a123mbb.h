
#ifndef _A123DFS_H_
#define _A123DFS_H_


#include "lpc_types.h"
#include "ccand_11xx.h"

//-------------------------
// Type Converters

#define num2mVolts(num) (uint32_t)((num + 2000)/2)
#define mVolts2Num(mvolts) (uint32_t)((mvolts - 1000) << 1)

#define num2degC(num) (int8_t)((num/2)-40)

//-------------------------
// BCM_CMD

#define BCM_CMD             0x50
#define BCM_CMD_DLC 		 3
#define BCM_REQUEST_TYPE_STD 0
#define BCM_REQUEST_TYPE_EXT 1
#define BCM_BALANCE_OFF 0x1FFE

typedef struct {
	uint8_t request_type;
	uint8_t request_id;
	bool balance;
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


//-------------------------
// MBB_EXT

#define MBB_EXT1 			0x300
#define MBB_EXT2 			0x400
#define MBB_EXT3 			0x500

#define MBB_EXT_MASK  		0xF00
#define MBB_EXT_MOD_ID_MASK 0x0FF
#define MBB_EXT_DLC 		8

#define MBB_GetExtRange(mode_id) (((mode_id & MBB_EXT_MASK) >> 8) - 3)
#define MBB_GetModID(mode_id) (mode_id & MBB_EXT_MOD_ID_MASK)

typedef struct {
	uint8_t id;
	uint16_t bal;
	uint16_t cell_mVolts[12];
} MBB_EXT_T;

int MBB_DecodeExt(MBB_EXT_T *contents, CCAN_MSG_OBJ_T *msg_obj);

/*
** Data Offsets
*/
#define MBB_RESP_MASK 0xF00
#define MBB_MOD_ID_MASK 0x0FF

#endif