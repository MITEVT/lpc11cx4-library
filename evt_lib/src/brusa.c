#include "brusa.h"

void Brusa_MakeCTL(NLG5_CTL_T *contents, CCAN_MSG_OBJ_T *msg_obj) {
	msg_obj->mode_id = NLG5_CTL;
	msg_obj->dlc = NLG5_CTL_DLC;
	msg_obj->data[0] = ((contents->enable & 1) << 7) | 
				  ((contents->clear_error & 1) << 6) | 
				  ((contents->ventilation_request & 1) << 5);
	uint16_t mains_dAmps = contents->max_mains_cAmps / 10;
	msg_obj->data[1] = mains_dAmps >> 8;
	msg_obj->data[2] = mains_dAmps & 0xFF;
	uint16_t out_dVolts = contents->output_mVolts / 100;
	msg_obj->data[3] = out_dVolts >> 8;
	msg_obj->data[4] = out_dVolts & 0xFF;
	uint16_t out_dAmps = contents->output_cAmps / 10;
	msg_obj->data[5] = out_dAmps >> 8;
	msg_obj->data[6] = out_dAmps & 0xFF;
}

int Brusa_DecodeStatus(NLG5_STATUS_T *contents, CCAN_MSG_OBJ_T *msg_obj) {
	if (msg_obj->dlc != NLG5_STATUS_DLC) {
		return -1;
	}

	contents->power_enabled = msg_obj->data[0] >> 7;
	contents->general_error_latch = (msg_obj->data[0] >> 6) & 1;
	contents->general_limit_warning = (msg_obj->data[0] >> 5) & 1;
	contents->fan_active = (msg_obj->data[0] >> 4) & 1;
	contents->mains_type = (msg_obj->data[0] >> 1) & 0x7;
	contents->control_pilot_detected = msg_obj->data[0] & 1;

	contents->bypass_detection = (msg_obj->data[1] >> 6) & 0x3;
	contents->limitation = ((msg_obj->data[1] & 0x1F) << 8) | msg_obj->data[2];

	return 0;
}

int Brusa_DecodeActI(NLG5_ACT_I_T *contents, CCAN_MSG_OBJ_T *msg_obj) {
	if (msg_obj->dlc != NLG5_ACT_I_DLC) {
		return -1;
	}

	contents->mains_cAmps = (msg_obj->data[0] << 8) | msg_obj->data[1];
	// Value originally in deciVolts
	contents->mains_mVolts = ((msg_obj->data[2] << 8) | msg_obj->data[3]) * 100; 
	// Value originally in deciVolts
	contents->output_mVolts = ((msg_obj->data[4] << 8) | msg_obj->data[5]) * 100; 
	contents->output_cAmps = (msg_obj->data[6] << 8) | msg_obj->data[7];

	return 0;
}

int Brusa_DecodeActII(NLG5_ACT_II_T *contents, CCAN_MSG_OBJ_T *msg_obj) {
	if (msg_obj->dlc != NLG5_ACT_II_DLC) {
		return -1;
	}

	contents->mains_current_max_pilot = (msg_obj->data[0] << 8) | msg_obj->data[1];
	contents->mains_current_max_power_ind = msg_obj->data[2];
	contents->aux_battery_voltage = msg_obj->data[3];
	contents->amp_hours_ext_shunt = (msg_obj->data[4] << 8) | msg_obj->data[5];
	contents->booster_output_current = (msg_obj->data[6] << 8) | msg_obj->data[7];

	return 0;
}

int Brusa_DecodeTemp(NLG5_TEMP_T *contents, CCAN_MSG_OBJ_T *msg_obj)  {
	if (msg_obj->dlc != NLG5_TEMP_DLC) {
		return -1;
	}

	contents->power_temp = (msg_obj->data[0] << 8) | msg_obj->data[1];
	contents->temp_1 = (msg_obj->data[2] << 8) | msg_obj->data[3];
	contents->temp_2 = (msg_obj->data[4] << 8) | msg_obj->data[5];
	contents->temp_3 = (msg_obj->data[6] << 8) | msg_obj->data[7];

	return 0;
}

