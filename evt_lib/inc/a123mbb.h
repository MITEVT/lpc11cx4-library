
#ifndef _A123DFS_H_
#define _A123DFS_H_

#define BCM_CMD             0x50
#define BCM_CMD_DLC 		 3
#define BCM_REQUEST_TYPE_STD 0
#define BCM_REQUEST_TYPE_EXT 1
#define BCM_BALANCE_OFF 0x1FFE

typedef struct {
	uint8_t request_type;
	uint8_t request_id;
	uint16_t balance_target;
} MBB_CMD_T;

void MBB_MakeCMD(MBB_CMD_T *contents, CCAN_MSG_OBJ_T *msg_obj) {
	msg_obj->mode_id = BCM_CMD;
	msg_obj->dlc = 3;
	msg_obj->data[0] = (contents->request_id << 4) | (contents->request_type);
	msg_obj->data[1] = (contents->balance_target >> 5);
	msg_obj->data[2] = (contents->balance_target & 0x1F) << 3;
}

/*
** MOD_STD
*/
#define MBB_STD             0x200
#define MBB_STD_MASK 		0xF00
#define MBB_STD_DLC 		8

typedef struct {
	uint8_t cell_overvolt;
	uint8_t cell_undervolt;
	uint8_t response_id;
	uint8_t temp;
	uint16_t mod_v_min;
	uint8_t temp_chn;
	uint16_t mod_v_max;
	uint8_t balance_c_count;
	uint16_t mod_v_avg;
	uint8_t voltage_mismatch;
} MBB_STD_T;

int MBB_DecodeStd(MBB_STD_T *contents, CCAN_MSG_OBJ_T *msg_obj) {
	if (msg_obj->dlc!= MBB_STD_DLC) {
		return -1;
	}

	contents->cell_overvolt = (msg_obj->data[0] >> 2) & 1;
	contents->cell_undervolt = (msg_obj->data[0] >> 3) & 1;
	contents->response_id = msg_obj->data[0] >> 4;
	contents->temp = msg_obj->data[1];
	contents->mod_v_min = ((msg_obj->data[2] << 8 )| (msg_obj->data[3] & 0xF8)) >> 3;
	contents->temp_chn = msg_obj->data[3] & 0x7;
	contents->mod_v_max = ((msg_obj->data[4] << 8) | (msg_obj->data[5] & 0xF8)) >> 3;
	contents->balance_c_count = msg_obj->data[5] & 0x7;
	contents->mod_v_avg = ((msg_obj->data[6] << 8) | (msg_obj->data[7] & 0xF8)) >> 3;
	contents->voltage_mismatch = msg_obj->data[7] & 0x7;

	return 0;
}


/*
** Data Offsets
*/
#define MOD_THERM_OFFSET -40
#define MOD_V_OFFSET 1

#endif