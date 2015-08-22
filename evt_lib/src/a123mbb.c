#include "a123mbb.h"


void MBB_MakeCMD(MBB_CMD_T *contents, CCAN_MSG_OBJ_T *msg_obj) {
	msg_obj->mode_id = BCM_CMD;
	msg_obj->dlc = 3;
	msg_obj->data[0] = (contents->request_id << 4) | (contents->request_type);
	uint16_t balance;
	if (contents->balance) {
		balance = mVolts2Num(contents->balance_target_mVolts);
	} else {
		balance = BCM_BALANCE_OFF;
	}
	
	msg_obj->data[1] = (balance >> 5) & 0xFF;
	msg_obj->data[2] = (balance & 0x1F) << 3;
}

int8_t MBB_DecodeStd(MBB_STD_T *contents, CCAN_MSG_OBJ_T *msg_obj) {
	if (msg_obj->dlc != MBB_STD_DLC) {
		return -1;
	} 

	contents->id = MBB_GetModID(msg_obj->mode_id);
	contents->cell_overvolt = (msg_obj->data[0] >> 2) & 1;
	contents->cell_undervolt = (msg_obj->data[0] >> 3) & 1;
	contents->response_id = msg_obj->data[0] >> 4;
	contents->temp_degC = msg_obj->data[1];
	contents->temp_degC = num2degC(contents->temp_degC);
	contents->mod_min_mVolts = ((msg_obj->data[2] << 8)| (msg_obj->data[3] & 0xF8)) >> 3;
	contents->mod_min_mVolts = num2mVolts(contents->mod_min_mVolts);
	contents->temp_chn = msg_obj->data[3] & 0x7;
	contents->mod_max_mVolts = ((msg_obj->data[4] << 8) | (msg_obj->data[5] & 0xF8)) >> 3;
	contents->mod_max_mVolts = num2mVolts(contents->mod_max_mVolts);
	contents->balance_c_count = msg_obj->data[5] & 0x7;
	// contents->mod_avg_mVolts = ((msg_obj->data[6] << 8) | (msg_obj->data[7] & 0xF8)) >> 3;
	// contents->mod_avg_mVolts = num2mVolts(contents->mod_avg_mVolts);
	contents->voltage_mismatch = msg_obj->data[7] & 0x4;

	return 0;
}

int8_t MBB_DecodeExt(MBB_EXT_T *contents, CCAN_MSG_OBJ_T *msg_obj) {
	if (msg_obj->dlc != MBB_EXT_DLC) {
		return -1;
	}

	uint8_t range =  MBB_GetExtRange(msg_obj->mode_id);

	if (range > 2) {
		return -1;
	}
	contents->id = MBB_GetModID(msg_obj->mode_id);
	contents->bal &= ~(0xF << range * 4);
	contents->bal |= ((msg_obj->data[1] & 0x04) >> 2) << range * 4;
	contents->bal |= ((msg_obj->data[3] & 0x04) >> 1) << range * 4;
	contents->bal |= ((msg_obj->data[5] & 0x04) >> 0) << range * 4;
	contents->bal |= ((msg_obj->data[7] & 0x04) << 1) << range * 4;
	contents->cell_mVolts[range * 4 + 0] = ((msg_obj->data[0] << 8) | (msg_obj->data[1] & 0xF8)) >> 3;
	contents->cell_mVolts[range * 4 + 0] = num2mVolts(contents->cell_mVolts[range * 4 + 0]);
	contents->cell_mVolts[range * 4 + 1] = ((msg_obj->data[2] << 8) | (msg_obj->data[3] & 0xF8)) >> 3;
	contents->cell_mVolts[range * 4 + 1] = num2mVolts(contents->cell_mVolts[range * 4 + 1]);
	contents->cell_mVolts[range * 4 + 2] = ((msg_obj->data[4] << 8) | (msg_obj->data[5] & 0xF8)) >> 3;
	contents->cell_mVolts[range * 4 + 2] = num2mVolts(contents->cell_mVolts[range * 4 + 2]);
	contents->cell_mVolts[range * 4 + 3] = ((msg_obj->data[6] << 8) | (msg_obj->data[7] & 0xF8)) >> 3;
	contents->cell_mVolts[range * 4 + 3] = num2mVolts(contents->cell_mVolts[range * 4 + 3]);
	

	return 0;
}


