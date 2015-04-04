#include "a123mbb.h"


void MBB_MakeCMD(MBB_CMD_T *contents, CCAN_MSG_OBJ_T *msg_obj) {
	msg_obj->mode_id = BCM_CMD;
	msg_obj->dlc = 3;
	msg_obj->data[0] = (contents->request_id << 4) | (contents->request_type);
	msg_obj->data[1] = (contents->balance_target >> 5);
	msg_obj->data[2] = (contents->balance_target & 0x1F) << 3;
}

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