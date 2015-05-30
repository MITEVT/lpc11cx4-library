// typedef struct CCAN_MSG_OBJ {
// 	uint32_t  mode_id;
// 	uint32_t  mask;
// 	uint8_t   data[8];
// 	uint8_t   dlc;
// 	uint8_t   msgobj;
// } CCAN_MSG_OBJ_T;

#ifndef _BRUSADFS_H_
#define _BRUSADFS_H_

#include "chip.h"

// ------------------------------------------------
// NLG5 Control

#define NLG5_CTL 0x618
#define NLG5_CTL_DLC 7

typedef struct {
	uint8_t enable; 				// 0 = Disable, 1 = Enable
	uint8_t clear_error;			// 0->1 = Clear error latch
	uint8_t ventilation_request;	// 0 = No Ventilation, 1 = Ventilation
	uint16_t max_mains_cAmps;
	uint32_t output_mVolts;
	uint16_t output_cAmps;
} NLG5_CTL_T;


void Brusa_MakeCTL(NLG5_CTL_T *contents, CCAN_MSG_OBJ_T *msg_obj);

// ------------------------------------------------
// NLG5 Status

#define NLG5_STATUS 0x610
#define NLG5_STATUS_DLC 4

#define NLG5_STATUS_MAINS_TYPE_EUROPE 4
#define NLG5_STATUS_MAINS_TYPE_USA1 2
#define NLG5_STATUS_MAINS_TYPE_USA2 1
#define NLG5_STATUS_MAINS_TYPE_UNKNOWN 0

#define NLG5_STATUS_BYPASS_TYPE_NONE 0
#define NLG5_STATUS_BYPASS_TYPE_DC 1
#define NLG5_STATUS_BYPASS_TYPE_AC_IN_PHASE 2
#define NLG5_STATUS_BYPASS_TYPE_AC_OUT_PHASE 3

#define NLG5_STATUS_LIMIT_TYPE_OUT_V 0x2000
#define NLG5_STATUS_LIMIT_TYPE_OUT_C 0x1000
#define NLG5_STATUS_LIMIT_TYPE_MAINS_C 0x800
#define NLG5_STATUS_LIMIT_TYPE_POW_IND 0x400
#define NLG5_STATUS_LIMIT_TYPE_CTRL_PILOT 0x200
#define NLG5_STATUS_LIMIT_TYPE_MAX_POW 0x100
#define NLG5_STATUS_LIMIT_TYPE_MAX_MAINS_C 0x80
#define NLG5_STATUS_LIMIT_TYPE_MAX_OUT_C 0x40
#define NLG5_STATUS_LIMIT_TYPE_MAX_OUT_V 0x20
#define NLG5_STATUS_LIMIT_TYPE_TEMP_CAP 0x10
#define NLG5_STATUS_LIMIT_TYPE_TEMP_POW 0x8
#define NLG5_STATUS_LIMIT_TYPE_TEMP_DIODE 0x4
#define NLG5_STATUS_LIMIT_TYPE_TEMP_TRNFRMR 0x2
#define NLG5_STATUS_LIMIT_TYPE_OUTV 0x1

typedef struct {
	uint8_t power_enabled;
	uint8_t general_error_latch;
	uint8_t general_limit_warning;
	uint8_t fan_active;
	uint8_t mains_type;
	uint8_t control_pilot_detected;
	uint8_t bypass_detection;
	uint16_t limitation;
} NLG5_STATUS_T;

int Brusa_DecodeStatus(NLG5_STATUS_T *contents, CCAN_MSG_OBJ_T *msg_obj);

// ------------------------------------------------
// NLG5 Actual Values 1

#define NLG5_ACT_I 0x611
#define NLG5_ACT_I_DLC 8

typedef struct {
	uint16_t mains_cAmps;
	uint32_t mains_mVolts;
	uint32_t output_mVolts;
	uint16_t output_cAmps;
} NLG5_ACT_I_T;

int Brusa_DecodeActI(NLG5_ACT_I_T *contents, CCAN_MSG_OBJ_T *msg_obj);

// ------------------------------------------------
// NLG5 Actual Values 2

#define NLG5_ACT_II 0x612
#define NLG5_ACT_II_DLC 8

typedef struct {
	uint16_t mains_current_max_pilot;		// DeciAmps
	uint8_t mains_current_max_power_ind;	// DeciAmps
	uint8_t aux_battery_voltage;			// DeciVolts
	uint16_t amp_hours_ext_shunt;			// DeciAmpHours
	uint16_t booster_output_current;		// CentiAmps
} NLG5_ACT_II_T;

int Brusa_DecodeActII(NLG5_ACT_II_T *contents, CCAN_MSG_OBJ_T *msg_obj);

// ------------------------------------------------
// NLG5 Temp Feedback

#define NLG5_TEMP 0x613
#define NLG5_TEMP_DLC 8

typedef struct {
	uint16_t power_temp;					// .1 C
	uint16_t temp_1; 						// .1 C
	uint16_t temp_2;						// .1 C
	uint16_t temp_3;						// .1 C
} NLG5_TEMP_T;

int Brusa_DecodeTemp(NLG5_TEMP_T *contents, CCAN_MSG_OBJ_T *msg_obj);


#endif


