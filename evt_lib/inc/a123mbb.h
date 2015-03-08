
#ifndef _A123DFS_H_
#define _A123DFS_H_

#define BCM_CMD             0x50
#define BCM_REQUEST_TYPE_STD 0
#define BCM_REQUEST_TYPE_EXT 1
#define BCM_BALANCE_OFF

typedef struct {
	uint8_t request_type;
	uint8_t request_id;
	uint16_t balance_target;
} MBB_CMD_T;

void MBB_MakeCMD(MBB_CMD_T *contents, CCAN_MSG_OBJ_T *msg_obj) {
	msg_obj->mode_id = BCM_CMD;
	msg_obj->dlc = 3
	msg_obj->data[0] = (contents->request_id << 4) | ;
}

/*
** MOD_STD
*/
#define MOD_STD             0x200


/*
** Data Offsets
*/
#define MOD_THERM_OFFSET -40
#define MOD_V_OFFSET 1

#endif