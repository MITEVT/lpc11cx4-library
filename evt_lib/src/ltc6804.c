#include "ltc6804.h"
#include "config.h"

/***************************************
		Private Variables
****************************************/

#define WAKE_BUF_LEN 40
static uint8_t wake_buf[WAKE_BUF_LEN];
static uint16_t owt_state;
static uint32_t owt_time;
static uint8_t owt_up_rx_buf[4][LTC6804_CALC_BUFFER_LEN(15)]; // [TODO] Move into ltc6804_state

#define _IS_ASLEEP(state, msTicks) (msTicks - state->last_message > T_SLEEP)
#define _IS_IDLE(state, msTicks) (msTicks - state->last_message > T_IDLE)
#define _IS_REFUP(state, msTicks) (msTicks - state->last_sleep_wake > T_REFUP)

/***************************************
		Private Function Prototypes
****************************************/

LTC6804_STATUS_T _command(LTC6804_CONFIG_T *config, LTC6804_STATE_T *state, uint16_t cmd, uint32_t msTicks);
LTC6804_STATUS_T _write(LTC6804_CONFIG_T *config, LTC6804_STATE_T *state, uint16_t cmd, uint32_t msTicks);
LTC6804_STATUS_T _read(LTC6804_CONFIG_T *config, LTC6804_STATE_T *state, uint16_t cmd, uint32_t msTicks);
LTC6804_STATUS_T _wake(LTC6804_CONFIG_T *config, LTC6804_STATE_T *state, uint32_t msTicks, bool force);
LTC6804_STATUS_T _set_balance_states(LTC6804_CONFIG_T *config, LTC6804_STATE_T *state, uint32_t msTicks);
uint16_t _calculate_pec(uint8_t *data, uint8_t len);
bool _check_st_results(LTC6804_CONFIG_T *config, LTC6804_STATE_T *state);
bool _check_owt(LTC6804_CONFIG_T *config, LTC6804_OWT_RES_T *res, uint8_t *c_up, uint8_t *c_down);

uint32_t divu10(uint32_t n) {
 uint32_t q, r;
 q = (n >> 1) + (n >> 2);
 q = q + (q >> 4);
 q = q + (q >> 8);
 q = q + (q >> 16);
 q = q >> 3;
 r = n - q*10;
 return q + ((r + 6) >> 4);
}

/***************************************
		Public Functions
****************************************/

LTC6804_STATUS_T LTC6804_Init(LTC6804_CONFIG_T *config, LTC6804_STATE_T *state, uint32_t msTicks) {
	Chip_GPIO_WriteDirBit(LPC_GPIO, config->cs_gpio, config->cs_pin, true);
	Chip_GPIO_SetPinState(LPC_GPIO, config->cs_gpio, config->cs_pin, true);

	Chip_SSP_Init(config->pSSP);
	Chip_SSP_SetBitRate(config->pSSP, config->baud);
	Chip_SSP_SetFormat(config->pSSP, SSP_BITS_8, SSP_FRAMEFORMAT_SPI, SSP_CLOCK_MODE3);
	Chip_SSP_SetMaster(config->pSSP, true);
	Chip_SSP_Enable(config->pSSP);

	uint16_t VUV = config->min_cell_mV * 10 / 16 - 1;
	uint16_t VOV = config->max_cell_mV * 10 / 16;

	state->xf->rx_data = state->rx_buf;
	state->cfg[0] = 0xBC; 
	state->cfg[1] = VUV & 0xFF;
	state->cfg[2] = (VUV & 0xF00) >> 8 | (VOV & 0xF) << 4; 
	state->cfg[3] = (VOV & 0xFF0) >> 4;
	state->cfg[4] = 0x00; 
	state->cfg[5] = 0x00; // [TODO] Consider using DCTO

	state->last_message = msTicks;
	state->wake_length = 3*config->baud/80000 + 1; // [TODO] Remember how this was calculated
	state->waiting = false;
	state->wait_time = LTC6804_ADCV_MODE_WAIT_TIMES[config->adc_mode];
	// state->wait_time = 7; // [TODO] Remove
	state->last_sleep_wake = msTicks;
	state->balancing = false;

	state->adc_status = LTC6804_ADC_NONE;

	owt_state = 0;

	memset(wake_buf, 0, WAKE_BUF_LEN);
	memset(state->bal_list, 0, sizeof(state->bal_list[0])*MAX_NUM_MODULES);

	_wake(config, state, msTicks, true);

	return LTC6804_WriteCFG(config, state, msTicks);
}

/* Clears Balance Bytes */
LTC6804_STATUS_T LTC6804_WriteCFG(LTC6804_CONFIG_T *config, LTC6804_STATE_T *state, uint32_t msTicks) {
	memcpy(state->tx_buf+4, state->cfg, 6);
	uint16_t pec = _calculate_pec(state->tx_buf+4, 6);
	state->tx_buf[10] = pec >> 8;
	state->tx_buf[11] = pec & 0xFF;

	int i;
	for (i = 0; i < config->num_modules; i++) {
		memcpy(state->tx_buf+4+8*i, state->tx_buf+4, 8);
	}

	return _write(config, state, WRCFG, msTicks);
}

bool LTC6804_VerifyCFG(LTC6804_CONFIG_T *config, LTC6804_STATE_T *state, uint32_t msTicks) {
	LTC6804_STATUS_T res = _read(config, state, RDCFG, msTicks);
	if (res != LTC6804_PASS) return false;

	int i;
	for (i = 0; i < config->num_modules; i++) {
		uint8_t *rx_ptr = state->rx_buf+4+8*i;
		if (rx_ptr[1] != state->cfg[1]) return false;
		if (rx_ptr[2] != state->cfg[2]) return false;
		if (rx_ptr[3] != state->cfg[3]) return false;
		if ((rx_ptr[5] & 0xF0) != (state->cfg[5] & 0xF0)) return false;
	}

	return true;
}

LTC6804_STATUS_T LTC6804_CVST(LTC6804_CONFIG_T *config, LTC6804_STATE_T *state, uint32_t msTicks) {
	if (state->adc_status == LTC6804_ADC_NONE) {
		state->adc_status = LTC6804_ADC_CVST;
	} else if (state->adc_status != LTC6804_ADC_CVST) {
		return LTC6804_WAITING;
	}

	if (_IS_ASLEEP(state, msTicks)) {
		_wake(config, state, msTicks, false);
		return LTC6804_WAITING;
	} else if (!_IS_REFUP(state, msTicks)) {
		return LTC6804_WAITING_REFUP;
	}

	if (!state->waiting) { // Need to send out self test command
		state->waiting = true;
		state->flight_time = msTicks;
		_command(config, state, (config->adc_mode << 7) | 0x227, msTicks);
		return LTC6804_WAITING;
	} else {
		if (msTicks - state->flight_time > state->wait_time) { // We've waited long enough
			state->waiting = false;
			state->adc_status = LTC6804_ADC_NONE;
			LTC6804_STATUS_T res;
			if ((res = _read(config, state, RDCVA, msTicks)) != LTC6804_PASS) return res;
			if (!_check_st_results(config, state)) return LTC6804_FAIL;
			if ((res = _read(config, state, RDCVB, msTicks)) != LTC6804_PASS) return res;
			if (!_check_st_results(config, state)) return LTC6804_FAIL;
			if ((res = _read(config, state, RDCVC, msTicks)) != LTC6804_PASS) return res;
			if (!_check_st_results(config, state)) return LTC6804_FAIL;
			if ((res = _read(config, state, RDCVD, msTicks)) != LTC6804_PASS) return res;
			if (!_check_st_results(config, state)) return LTC6804_FAIL;
			return LTC6804_PASS;
		} else { // Keep Waiting
			return LTC6804_WAITING;
		}
	}
}

#define LENGTH 35 // [TODO] Put in config

// [TODO] Don't break at module fail (cycle through all modules)
LTC6804_STATUS_T LTC6804_OpenWireTest(LTC6804_CONFIG_T *config, LTC6804_STATE_T *state, LTC6804_OWT_RES_T *res, uint32_t msTicks) {

	if (state->adc_status == LTC6804_ADC_NONE) {
		state->adc_status = LTC6804_ADC_OWT;
	} else if (state->adc_status != LTC6804_ADC_OWT) {
		return LTC6804_WAITING;
	}

	int i;

	if (msTicks - owt_time < state->wait_time) {
		return LTC6804_WAITING;
	}

	if (owt_state <= LENGTH) {
		owt_state++;
		owt_time = msTicks;
		_command(config, state, (config->adc_mode << 7) | 0x268, msTicks);
		return LTC6804_WAITING; 
	} else if (owt_state == LENGTH + 1) {
		uint8_t *normal_rx_ptr = state->rx_buf;
		state->rx_buf = owt_up_rx_buf[0];
		LTC6804_STATUS_T r;
		r = _read(config, state, RDCVA, msTicks);
		if (r != LTC6804_PASS) {state->rx_buf = normal_rx_ptr; return r;}
		for (i = 0; i < config->num_modules; i++) {
			uint8_t *rx_ptr = owt_up_rx_buf[0] + 4 + 8 * i;
			if (rx_ptr[0] == 0 && rx_ptr[1] == 0) {
				state->rx_buf = normal_rx_ptr;
				owt_state = 0;
				res->failed_module = i;
				res->failed_wire = 0;
				return LTC6804_FAIL;
			}
		}
		state->rx_buf = owt_up_rx_buf[1];
		r = _read(config, state, RDCVB, msTicks);
		if (r != LTC6804_PASS) {state->rx_buf = normal_rx_ptr; return r;}
		state->rx_buf = owt_up_rx_buf[2];
		r = _read(config, state, RDCVC, msTicks);
		if (r != LTC6804_PASS) {state->rx_buf = normal_rx_ptr; return r;}
		state->rx_buf = owt_up_rx_buf[3];
		r = _read(config, state, RDCVD, msTicks);
		if (r != LTC6804_PASS) {state->rx_buf = normal_rx_ptr; return r;}
		state->rx_buf = normal_rx_ptr;
		owt_state++;
		owt_time = msTicks;
		_command(config, state, (config->adc_mode << 7) | 0x228, msTicks);
		return LTC6804_WAITING;
	} else if (owt_state <= LENGTH*2) {
		owt_state++;
		owt_time = msTicks;
		_command(config, state, (config->adc_mode << 7) | 0x228, msTicks);
		return LTC6804_WAITING;
	} else if (owt_state == LENGTH*2 + 1) {
		state->adc_status = LTC6804_ADC_NONE;
		LTC6804_STATUS_T r;
		for (i = 0; i < 4; i++) {
			r = _read(config, state, RDCVA+2*i, msTicks);
			if (r != LTC6804_PASS) {return r;}
			if (!_check_owt(config, res, owt_up_rx_buf[i], state->rx_buf)) {
				owt_state = 0;
				res->failed_wire += 3*i;
				return LTC6804_FAIL;
			}
		}
		for (i = 0; i < config->num_modules; i++) {
			uint8_t *rx_ptr = state->rx_buf + 4 + 8 * i;
			if (rx_ptr[5] == 0 && rx_ptr[4] == 0) {
				owt_state = 0;
				res->failed_module = i;
				res->failed_wire = 12;
				return LTC6804_FAIL;
			}

		}
		owt_state = 0;
		owt_time = msTicks;
		return LTC6804_PASS;
	} else {
		return LTC6804_FAIL;
	}
}

// Once updates on change
LTC6804_STATUS_T LTC6804_UpdateBalanceStates(LTC6804_CONFIG_T *config, LTC6804_STATE_T *state, bool *balance_req, uint32_t msTicks) {
	bool *bal_ptr = balance_req;
	bool change = false;
	int i, j;
	state->balancing = false;
	for (i = 0; i < config->num_modules; i++) {
		uint16_t bal_temp = 0;
		for (j = 0; j < config->module_cell_count[i]; j++) {
			bal_temp |= bal_ptr[0] << j;
			bal_ptr++;
		}
		if (bal_temp != state->bal_list[i]) {
			change = true;
			state->bal_list[i] = bal_temp;
		}
		if (state->bal_list[i]) state->balancing = true;
	}

	if (change) {
		return _set_balance_states(config, state, msTicks);
	} else {
		return LTC6804_PASS;
	}
}

LTC6804_STATUS_T LTC6804_SetGPIOState(LTC6804_CONFIG_T *config, LTC6804_STATE_T *state, uint8_t gpio, bool val, uint32_t msTicks) {
	// valid gpio range: 1 <= gpio <= 5
	if (gpio > 5 || gpio < 1) {
		return LTC6804_FAIL;
	}

	if ((val & 1) == ((state->cfg[0] >> (2 + gpio)) & 1)) {
		// No Change
		return LTC6804_PASS;
	} else {
		if (val)
			state->cfg[0] |= 1 << (gpio + 2);
		else
			state->cfg[0] &= ~(1 << (gpio + 2));

		return _set_balance_states(config, state, msTicks);
	}
}

LTC6804_STATUS_T LTC6804_GPIORiseFall(LTC6804_CONFIG_T * config, LTC6804_STATE_T * state,
		uint8_t gpio, uint32_t msTicks) {
	// TODO: should I set the GPIO to 0 at the beginning?
	// TODO: Think about what to return
	LTC6804_SetGPIOState(config, state, gpio, 1, msTicks);
	return LTC6804_SetGPIOState(config, state, gpio, 0, msTicks);
}

// [TODO] Clear cell votlages and only return pass when recieving not all FF
LTC6804_STATUS_T LTC6804_GetCellVoltages(LTC6804_CONFIG_T *config, LTC6804_STATE_T *state, LTC6804_ADC_RES_T *res, uint32_t msTicks) {
	if (state->adc_status == LTC6804_ADC_NONE) {
		state->adc_status = LTC6804_ADC_GCV;
	} else if (state->adc_status != LTC6804_ADC_GCV) {
		return LTC6804_WAITING;
	}

	if (_IS_ASLEEP(state, msTicks)) {
		_wake(config, state, msTicks, false);
		return LTC6804_WAITING;
	} else if (!_IS_REFUP(state, msTicks)) {
		return LTC6804_WAITING_REFUP;
	}

	if (!state->waiting) { // Need to send out self test command
		state->waiting = true;
		state->flight_time = msTicks;
		_command(config, state, (config->adc_mode << 7) | 0x270, msTicks); // ADCV, All Channels, DCP=1
		return LTC6804_WAITING;
	} else {
		// [TODO] make >=
		if (msTicks - state->flight_time > state->wait_time) { // We've waited long enough
			state->waiting = false;
			int i, j;
			uint32_t *vol_ptr = res->cell_voltages_mV;
			uint32_t min = UINT32_MAX;
			uint32_t max = 0;
			uint16_t failed_read = 0xFFFF;
			// Read a cell voltage group
				// For each module, calculate head of module voltage group
				// vol_ptr is pointer to head of module cell group voltages
			LTC6804_STATUS_T r;
			r = _read(config, state, RDCVA, msTicks);
			if (r == LTC6804_PEC_ERROR) return r;
			for (i = 0; i < config->num_modules; i++) {
				uint8_t *rx_ptr = state->rx_buf + 4 + 8 * i;
				for (j = 0; j < config->module_cell_count[i] && j < 3; j++) {
					uint16_t vol = (rx_ptr[2 * j + 1] << 8 | rx_ptr[2 * j]);
					failed_read &= vol;
					vol_ptr[j] = divu10(vol);
					max = (vol_ptr[j] > max) ? vol_ptr[j] : max;
					min = (vol_ptr[j] < min) ? vol_ptr[j] : min;
				}
				vol_ptr = vol_ptr + config->module_cell_count[i];
			}

			r = _read(config, state, RDCVB, msTicks);
			if (r == LTC6804_PEC_ERROR) return r;
			vol_ptr = res->cell_voltages_mV;
			for (i = 0; i < config->num_modules; i++) {
				uint8_t *rx_ptr = state->rx_buf + 4 + 8 * i;
				for (j = 0; j + 3 < config->module_cell_count[i] && j < 3; j++) {
					uint16_t vol = (rx_ptr[2 * j + 1] << 8 | rx_ptr[2 * j]);
					failed_read &= vol;
					vol_ptr[j + 3] = divu10(vol);
					max = (vol_ptr[j + 3] > max) ? vol_ptr[j + 3] : max;
					min = (vol_ptr[j + 3] < min) ? vol_ptr[j + 3] : min;
				} 
				vol_ptr = vol_ptr + config->module_cell_count[i];
			}

			r = _read(config, state, RDCVC, msTicks);
			if (r == LTC6804_PEC_ERROR) return r;
			vol_ptr = res->cell_voltages_mV;
			for (i = 0; i < config->num_modules; i++) {
				uint8_t *rx_ptr = state->rx_buf + 4 + 8 * i;
				for (j = 0; j + 6 < config->module_cell_count[i] && j < 3; j++) {
					uint16_t vol = (rx_ptr[2 * j + 1] << 8 | rx_ptr[2 * j]);
					failed_read &= vol;
					vol_ptr[j + 6] = divu10(vol);
					max = (vol_ptr[j + 6] > max) ? vol_ptr[j + 6] : max;
					min = (vol_ptr[j + 6] < min) ? vol_ptr[j + 6] : min;
				} 
				vol_ptr = vol_ptr + config->module_cell_count[i];
			}
			
			r = _read(config, state, RDCVD, msTicks);
			if (r == LTC6804_PEC_ERROR) return r;
			vol_ptr = res->cell_voltages_mV;
			for (i = 0; i < config->num_modules; i++) {
				uint8_t *rx_ptr = state->rx_buf + 4 + 8 * i;
				for (j = 0; j + 9 < config->module_cell_count[i] && j < 3; j++) {
					uint16_t vol = (rx_ptr[2 * j + 1] << 8 | rx_ptr[2 * j]);
					failed_read &= vol;
					vol_ptr[j + 9] = divu10(vol);
					max = (vol_ptr[j + 9] > max) ? vol_ptr[j + 9] : max;
					min = (vol_ptr[j + 9] < min) ? vol_ptr[j + 9] : min;
				} 
				vol_ptr = vol_ptr + config->module_cell_count[i];
			}

			state->adc_status = LTC6804_ADC_NONE;

			if (failed_read == 0xFFFF) {
				return LTC6804_FAIL;
			}

			res->pack_cell_max_mV = max;
			res->pack_cell_min_mV = min;
			return LTC6804_PASS;
		} else { // Keep Waiting
			return LTC6804_WAITING;
		}
	}
}

LTC6804_STATUS_T LTC6804_ClearCellVoltages(LTC6804_CONFIG_T *config, LTC6804_STATE_T *state, uint32_t msTicks) {
	return _command(config, state, CLRCELL, msTicks);
}

LTC6804_STATUS_T LTC6804_GetGPIOVoltages(LTC6804_CONFIG_T *config, LTC6804_STATE_T *state, uint32_t *res, uint32_t msTicks) {
	if (state->adc_status == LTC6804_ADC_NONE) {
		state->adc_status = LTC6804_ADC_GPIO;
	} else if (state->adc_status != LTC6804_ADC_GPIO) {
		return LTC6804_WAITING;
	}

	if (_IS_ASLEEP(state, msTicks)) {
		_wake(config, state, msTicks, false);
		return LTC6804_WAITING;
	} else if (!_IS_REFUP(state, msTicks)) {
		return LTC6804_WAITING_REFUP;
	}

	if (!state->waiting) { // Need to send out self test command
		state->waiting = true;
		state->flight_time = msTicks;
		_command(config, state, (config->adc_mode << 7) | 0x460, msTicks); // ADAX, All Channels
		return LTC6804_WAITING;
	} else {
		if (msTicks - state->flight_time >= state->wait_time) { // We've waited long enough
			state->waiting = false;
			int i;
			uint32_t *vol_ptr = res;
			uint16_t failed_read = 0xFFFF;
			// Read a cell voltage group
				// For each module, calculate head of module voltage group
				// vol_ptr is pointer to head of module cell group voltages
			LTC6804_STATUS_T r;
			r = _read(config, state, RDAUXA, msTicks);
			if (r == LTC6804_PEC_ERROR) return r;
			for (i = 0; i < config->num_modules; i++) {
				uint8_t *rx_ptr = state->rx_buf + 4 + 8 * i;
				vol_ptr[0] = rx_ptr[0] | (rx_ptr[1] << 8); failed_read &= vol_ptr[0]; vol_ptr[0] = divu10(vol_ptr[0]);
				vol_ptr[1] = rx_ptr[2] | (rx_ptr[3] << 8); failed_read &= vol_ptr[0]; vol_ptr[1] = divu10(vol_ptr[1]);
				vol_ptr[2] = rx_ptr[4] | (rx_ptr[5] << 8); failed_read &= vol_ptr[0]; vol_ptr[2] = divu10(vol_ptr[2]);
				vol_ptr = vol_ptr + 5; // Num GPIOs
			}

			r = _read(config, state, RDAUXB, msTicks);
			if (r == LTC6804_PEC_ERROR) return r;
			for (i = 0; i < config->num_modules; i++) {
				uint8_t *rx_ptr = state->rx_buf + 4 + 8 * i;
				vol_ptr[3] = rx_ptr[0] | (rx_ptr[1] << 8); failed_read &= vol_ptr[0]; vol_ptr[3] = divu10(vol_ptr[3]);
				vol_ptr[4] = rx_ptr[2] | (rx_ptr[3] << 8); failed_read &= vol_ptr[0]; vol_ptr[4] = divu10(vol_ptr[4]);
				vol_ptr = vol_ptr + 5; // Num GPIOs
			}

			state->adc_status = LTC6804_ADC_NONE;

			if (failed_read == 0xFFFF) {
				return LTC6804_FAIL;
			}

			return LTC6804_PASS;
		} else { // Keep Waiting
			return LTC6804_WAITING;
		}
	}
}
/***************************************
		Private Functions
****************************************/

LTC6804_STATUS_T _command(LTC6804_CONFIG_T *config, LTC6804_STATE_T *state, uint16_t cmd, uint32_t msTicks) {
	_wake(config, state, msTicks, false);

	state->tx_buf[0] = cmd >> 8;
	state->tx_buf[1] = cmd & 0xFF;
	uint16_t pec = _calculate_pec(state->tx_buf, 2);
	state->tx_buf[2] = pec >> 8;
	state->tx_buf[3] = pec & 0xFF;

	state->last_message = msTicks;
	Chip_GPIO_SetPinState(LPC_GPIO, config->cs_gpio, config->cs_pin, false);
	Chip_SSP_WriteFrames_Blocking(config->pSSP, state->tx_buf, 4);
	Chip_GPIO_SetPinState(LPC_GPIO, config->cs_gpio, config->cs_pin, true);

	return LTC6804_PASS;
}

LTC6804_STATUS_T _write(LTC6804_CONFIG_T *config, LTC6804_STATE_T *state, uint16_t cmd, uint32_t msTicks) {
	_wake(config, state, msTicks, false);

	state->tx_buf[0] = cmd >> 8;
	state->tx_buf[1] = cmd & 0xFF;
	uint16_t pec = _calculate_pec(state->tx_buf, 2);
	state->tx_buf[2] = pec >> 8;
	state->tx_buf[3] = pec & 0xFF;

	state->last_message = msTicks;
	Chip_GPIO_SetPinState(LPC_GPIO, config->cs_gpio, config->cs_pin, false);
	Chip_SSP_WriteFrames_Blocking(config->pSSP, state->tx_buf, 4+8*config->num_modules);
	Chip_GPIO_SetPinState(LPC_GPIO, config->cs_gpio, config->cs_pin, true);

	return LTC6804_PASS;
}

LTC6804_STATUS_T _read(LTC6804_CONFIG_T *config, LTC6804_STATE_T *state, uint16_t cmd, uint32_t msTicks) {
	_wake(config, state, msTicks, false);

	state->tx_buf[0] = cmd >> 8;
	state->tx_buf[1] = cmd & 0xFF;
	uint16_t pec = _calculate_pec(state->tx_buf, 2);
	state->tx_buf[2] = pec >> 8;
	state->tx_buf[3] = pec & 0xFF;

	memset(state->tx_buf+4, 0, 8*config->num_modules);

	state->xf->length = 4 + 8 * config->num_modules; state->xf->rx_cnt = 0; state->xf->tx_cnt = 0;
	state->xf->rx_data = state->rx_buf;
	state->xf->tx_data = state->tx_buf;

	state->last_message = msTicks;
	Chip_GPIO_SetPinState(LPC_GPIO, config->cs_gpio, config->cs_pin, false);
	Chip_SSP_RWFrames_Blocking(config->pSSP, state->xf);
	Chip_GPIO_SetPinState(LPC_GPIO, config->cs_gpio, config->cs_pin, true);


	// Check PECs
	int i;
	for (i = 0; i < config->num_modules; i++) {
		uint8_t *rx_ptr = state->rx_buf + 4 + 8 * i;
		pec = _calculate_pec(rx_ptr, 6);
		if (pec != (rx_ptr[6] << 8 | rx_ptr[7])) return LTC6804_PEC_ERROR;
	}

	return LTC6804_PASS;
}

// [NOTE] If timing is fucked look here
LTC6804_STATUS_T _wake(LTC6804_CONFIG_T *config, LTC6804_STATE_T *state, uint32_t msTicks, bool force) {
	// if (msTicks - state->last_message < 4 && !force) return LTC6804_PASS;

	if (msTicks - state->last_message < T_IDLE && !force) return LTC6804_PASS;


	uint32_t wake;
	bool waking = msTicks - state->last_message >= T_SLEEP;

	wake = (waking || force) ? state->wake_length : 2;
	state->last_sleep_wake = (waking) ? msTicks : state->last_sleep_wake;


	// [TODO] Consider moving to _write or _command or _generic_write
	state->last_message = msTicks;
	Chip_GPIO_SetPinState(LPC_GPIO, config->cs_gpio, config->cs_pin, false);
	Chip_SSP_WriteFrames_Blocking(config->pSSP, wake_buf, wake);
	Chip_GPIO_SetPinState(LPC_GPIO, config->cs_gpio, config->cs_pin, true);

	// If you fall asleep, write configuration again pls
	if (waking) LTC6804_WriteCFG(config, state, msTicks); // Wake will be skipped bc last_message updated
	if (waking && state->balancing) _set_balance_states(config, state, msTicks);

	return LTC6804_PASS;
}

LTC6804_STATUS_T _set_balance_states(LTC6804_CONFIG_T *config, LTC6804_STATE_T *state, uint32_t msTicks) {
	int i;
	// [TODO] ensure that when sending out you have correct order
	for (i = 0; i < config->num_modules; i++) {
		uint8_t *tx_ptr = state->tx_buf + 4 + 8 * i;
		tx_ptr[0] = state->cfg[0]; tx_ptr[1] = state->cfg[1];
		tx_ptr[2] = state->cfg[2]; tx_ptr[3] = state->cfg[3];
		tx_ptr[4] = state->bal_list[config->num_modules - i - 1] & 0xFF;
		tx_ptr[5] = (state->cfg[5] & 0xF0) | (state->bal_list[config->num_modules - i - 1] >> 8);
		uint16_t pec = _calculate_pec(tx_ptr, 6);
		tx_ptr[6] = pec >> 8;
		tx_ptr[7] = pec & 0xFF;
	}
	return _write(config, state, WRCFG, msTicks);
}

static const unsigned int crc15Table[256] = {
	0x0000, 0xc599, 0xceab, 0xb32, 0xd8cf, 0x1d56, 0x1664, 0xd3fd, 0xf407, 0x319e, 0x3aac,  //!<precomputed CRC15 Table
	0xff35, 0x2cc8, 0xe951, 0xe263, 0x27fa, 0xad97, 0x680e, 0x633c, 0xa6a5, 0x7558, 0xb0c1, 
	0xbbf3, 0x7e6a, 0x5990, 0x9c09, 0x973b, 0x52a2, 0x815f, 0x44c6, 0x4ff4, 0x8a6d, 0x5b2e,
	0x9eb7, 0x9585, 0x501c, 0x83e1, 0x4678, 0x4d4a, 0x88d3, 0xaf29, 0x6ab0, 0x6182, 0xa41b,
	0x77e6, 0xb27f, 0xb94d, 0x7cd4, 0xf6b9, 0x3320, 0x3812, 0xfd8b, 0x2e76, 0xebef, 0xe0dd, 
	0x2544, 0x2be, 0xc727, 0xcc15, 0x98c, 0xda71, 0x1fe8, 0x14da, 0xd143, 0xf3c5, 0x365c, 
	0x3d6e, 0xf8f7,0x2b0a, 0xee93, 0xe5a1, 0x2038, 0x7c2, 0xc25b, 0xc969, 0xcf0, 0xdf0d, 
	0x1a94, 0x11a6, 0xd43f, 0x5e52, 0x9bcb, 0x90f9, 0x5560, 0x869d, 0x4304, 0x4836, 0x8daf,
	0xaa55, 0x6fcc, 0x64fe, 0xa167, 0x729a, 0xb703, 0xbc31, 0x79a8, 0xa8eb, 0x6d72, 0x6640,
	0xa3d9, 0x7024, 0xb5bd, 0xbe8f, 0x7b16, 0x5cec, 0x9975, 0x9247, 0x57de, 0x8423, 0x41ba,
	0x4a88, 0x8f11, 0x57c, 0xc0e5, 0xcbd7, 0xe4e, 0xddb3, 0x182a, 0x1318, 0xd681, 0xf17b, 
	0x34e2, 0x3fd0, 0xfa49, 0x29b4, 0xec2d, 0xe71f, 0x2286, 0xa213, 0x678a, 0x6cb8, 0xa921, 
	0x7adc, 0xbf45, 0xb477, 0x71ee, 0x5614, 0x938d, 0x98bf, 0x5d26, 0x8edb, 0x4b42, 0x4070, 
	0x85e9, 0xf84, 0xca1d, 0xc12f, 0x4b6, 0xd74b, 0x12d2, 0x19e0, 0xdc79, 0xfb83, 0x3e1a, 0x3528, 
	0xf0b1, 0x234c, 0xe6d5, 0xede7, 0x287e, 0xf93d, 0x3ca4, 0x3796, 0xf20f, 0x21f2, 0xe46b, 0xef59, 
	0x2ac0, 0xd3a, 0xc8a3, 0xc391, 0x608, 0xd5f5, 0x106c, 0x1b5e, 0xdec7, 0x54aa, 0x9133, 0x9a01, 
	0x5f98, 0x8c65, 0x49fc, 0x42ce, 0x8757, 0xa0ad, 0x6534, 0x6e06, 0xab9f, 0x7862, 0xbdfb, 0xb6c9, 
	0x7350, 0x51d6, 0x944f, 0x9f7d, 0x5ae4, 0x8919, 0x4c80, 0x47b2, 0x822b, 0xa5d1, 0x6048, 0x6b7a, 
	0xaee3, 0x7d1e, 0xb887, 0xb3b5, 0x762c, 0xfc41, 0x39d8, 0x32ea, 0xf773, 0x248e, 0xe117, 0xea25, 
	0x2fbc, 0x846, 0xcddf, 0xc6ed, 0x374, 0xd089, 0x1510, 0x1e22, 0xdbbb, 0xaf8, 0xcf61, 0xc453, 
	0x1ca, 0xd237, 0x17ae, 0x1c9c, 0xd905, 0xfeff, 0x3b66, 0x3054, 0xf5cd, 0x2630, 0xe3a9, 0xe89b, 
	0x2d02, 0xa76f, 0x62f6, 0x69c4, 0xac5d, 0x7fa0, 0xba39, 0xb10b, 0x7492, 0x5368, 0x96f1, 0x9dc3, 
	0x585a, 0x8ba7, 0x4e3e, 0x450c, 0x8095
}; 

uint16_t _calculate_pec(uint8_t *data, uint8_t len) {
	uint16_t remainder,addr;
	
	remainder = 16;//initialize the PEC
	uint8_t i;
	for(i = 0; i<len;i++) // loops for each byte in data array
	{
		addr = ((remainder>>7)^data[i])&0xff;//calculate PEC table address 
		remainder = (remainder<<8)^crc15Table[addr];
	}
	return(remainder*2);//The CRC15 has a 0 in the LSB so the remainder must be multiplied by 2
}

bool _check_st_results(LTC6804_CONFIG_T *config, LTC6804_STATE_T *state) {
	int i;
	for (i = 0; i < config->num_modules; i++) {
		uint8_t *rx_ptr = state->rx_buf+4+8*i;
		if ((rx_ptr[1] << 8 | rx_ptr[0]) != LTC6804_SELF_TEST_RES[config->adc_mode]) return false;
		if ((rx_ptr[3] << 8 | rx_ptr[2]) != LTC6804_SELF_TEST_RES[config->adc_mode]) return false;
		if ((rx_ptr[5] << 8 | rx_ptr[4]) != LTC6804_SELF_TEST_RES[config->adc_mode]) return false;
	}

	return true;
}

bool _check_owt(LTC6804_CONFIG_T *config, LTC6804_OWT_RES_T *res, uint8_t *c_up, uint8_t *c_down) {
	int32_t up, down;
	int i;
	for (i = 0; i < config->num_modules; i++) {
		uint8_t *down_ptr = c_down + 4 + 8 * i;
		uint8_t *up_ptr = c_up + 4 + 8 * i;
		up = (up_ptr[1] << 8) | up_ptr[0];
		down = (down_ptr[1] << 8) | down_ptr[0];
		if (up - down < -4000) {
			res->failed_module = i;
			res->failed_wire = 0;
			return false;
		}
		up = (up_ptr[3] << 8) | up_ptr[2];
		down = (down_ptr[3] << 8) | down_ptr[2];
		if (up - down < -4000) {
			res->failed_module = i;
			res->failed_wire = 1;
			return false;
		}
		up = (up_ptr[5] << 8) | up_ptr[4];
		down = (down_ptr[5] << 8) | down_ptr[4];
		if (up - down < -4000) {
			res->failed_module = i;
			res->failed_wire = 2;
			return false;
		}
	}
	return true;
}

// void LTC6804_OpenWireTestCmd(uint8_t pup_bit, uint32_t msTicks) {
// 	Tx_Buf[0] = 0x03;
//     if(pup_bit == 0) {
// 	    Tx_Buf[1] = 0x28;
//     } else {
// 	    Tx_Buf[1] = 0x68;
//     }
// 	uint16_t pec = _calculate_pec(Tx_Buf, 2);
// 	Tx_Buf[2] = pec >> 8;
// 	Tx_Buf[3] = pec & 0xFF;

// 	_wake(msTicks);

// 	_last_message = msTicks;
// 	Chip_GPIO_SetPinState(LPC_GPIO, _cs_gpio, _cs_pin, false);
// 	Chip_SSP_WriteFrames_Blocking(_pSSP, Tx_Buf, 4);
// 	Chip_GPIO_SetPinState(LPC_GPIO, _cs_gpio, _cs_pin, true);
// }
