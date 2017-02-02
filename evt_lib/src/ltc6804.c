#include "ltc6804.h"

/***************************************
		Private Variables
****************************************/

#define WAKE_BUF_LEN 40
static uint8_t wake_buf[WAKE_BUF_LEN];
static uint16_t owt_state;
static uint32_t owt_time;
static uint8_t owt_up_rx_buf[4][LTC6804_CALC_BUFFER_LEN(15)]; 

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
	state->wait_time = LTC6804_ADC_MODE_WAIT_TIMES[config->adc_mode];
	// state->wait_time = 7; // [TODO] Remove
	state->last_sleep_wake = msTicks;
	state->balancing = false;

	owt_state = 0;

	memset(wake_buf, 0, WAKE_BUF_LEN);

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

static uint32_t cell_voltages[2][15*12];
static LTC6804_ADC_RES_T adc_res;
static uint16_t cmd;

#define LENGTH 35

// [TODO] Test with pin toggling
LTC6804_STATUS_T LTC6804_OpenWireTest(LTC6804_CONFIG_T *config, LTC6804_STATE_T *state, volatile uint32_t *msTicks) {
	if (_IS_ASLEEP(state, *msTicks)) {
		_wake(config, state, *msTicks, false);
		return LTC6804_WAITING;
	} else if (!_IS_REFUP(state, *msTicks)) {
		// Chip_UART_SendBlocking(LPC_USART, "ref\r\n", 5);
		return LTC6804_WAITING_REFUP;
	}

	int i;
	LTC6804_STATUS_T r;
	if (owt_state <= LENGTH) {
		if (*msTicks - owt_time > state->wait_time) {
			// if (owt_state == 0){
			// 	Chip_GPIO_SetPinState(LPC_GPIO, 1,3,1 - Chip_GPIO_GetPinState(LPC_GPIO, 1,3));

			// }
			owt_state++;
			owt_time = *msTicks;
			r = _command(config, state, (config->adc_mode << 7) | 0x268, *msTicks);
		}
		return LTC6804_WAITING;
	} else if (owt_state == LENGTH+1) {
		adc_res.cell_voltages_mV = cell_voltages[0];
		cmd = 0x268;
		r = LTC6804_GetCellVoltages(config, state, &adc_res, *msTicks);
		if (r == LTC6804_PASS) {
			owt_time = *msTicks;
			owt_state++;
		}
		return LTC6804_WAITING;
	} else if (owt_state <= LENGTH*2) {
		if (*msTicks - owt_time > state->wait_time) {
			owt_state++;
			owt_time = *msTicks;
			_command(config, state, (config->adc_mode << 7) | 0x228, *msTicks);
		}
		return LTC6804_WAITING;
	} else if (owt_state == LENGTH*2+1) {

		adc_res.cell_voltages_mV = cell_voltages[1];
		cmd = 0x228;
		r = LTC6804_GetCellVoltages(config, state, &adc_res, *msTicks);
		if (r == LTC6804_PASS) {
			owt_time = *msTicks;
			owt_state = 0;
			if (cell_voltages[0][0] == 0) {
				return LTC6804_FAIL;
			}
			if (cell_voltages[1][11] == 0) {
				return LTC6804_FAIL;
			}
			// uint8_t str[10];
			// for (i = 0; i < 12; i++) {
			// 	itoa((int32_t)(cell_voltages[0][i]), str, 10);
			// 	Chip_UART_SendBlocking(LPC_USART, str, strlen(str));
			// 	Chip_UART_SendBlocking(LPC_USART, ", ", 2);
			// 	itoa((int32_t)(cell_voltages[1][i]), str, 10);
			// 	Chip_UART_SendBlocking(LPC_USART, str, strlen(str));
			// 	Chip_UART_SendBlocking(LPC_USART, "\n\r ", 2);
			// }
			// Chip_UART_SendBlocking(LPC_USART, "\n\r ", 2);

			for (i = 1; i < 12; i++) {
				// itoa((int32_t)(cell_voltages[0][i]) - (int32_t)(cell_voltages[1][i]), str, 10);
				// Chip_UART_SendBlocking(LPC_USART, str, strlen(str));
				// Chip_UART_SendBlocking(LPC_USART, ", ", 3);
				if ((int32_t)(cell_voltages[0][i]) - (int32_t)(cell_voltages[1][i]) < -400) {
					// itoa((int32_t)(cell_voltages[0][i]) - (int32_t)(cell_voltages[1][i]), str, 10);
					// Chip_UART_SendBlocking(LPC_USART, str, strlen(str));
					// Chip_UART_SendBlocking(LPC_USART, " (", 2);
					// itoa(i, str, 10);
					// Chip_UART_SendBlocking(LPC_USART, str, strlen(str));
					// Chip_UART_SendBlocking(LPC_USART, ")", 1);
					return LTC6804_FAIL;
				}
				// Board_Print_BLOCKING("\r\n");
			}
			
			
			return LTC6804_PASS;
		} else if (r == LTC6804_PEC_ERROR) {
			return r;
		}
		return LTC6804_WAITING;
	} else {
		return LTC6804_FAIL;
	}
}

// [TODO] Return Cell and Module Failure
// [TODO] Lock out ADC if running
// [TODO] Lock out if ADC running
// LTC6804_STATUS_T LTC6804_OpenWireTest(LTC6804_CONFIG_T *config, LTC6804_STATE_T *state, uint32_t msTicks) {
// 	if (_IS_ASLEEP(state, msTicks)) {
// 		_wake(config, state, msTicks, false);
// 		return LTC6804_WAITING;
// 	} else if (!_IS_REFUP(state, msTicks)) {
// 		return LTC6804_WAITING_REFUP;
// 	}

// 	int i;
// 	switch(owt_state) {
// 		case 0:
// 			owt_state++;
// 			owt_time = msTicks;
// 			_command(config, state, (config->adc_mode << 7) | 0x268, msTicks);
// 			return LTC6804_WAITING;
// 		case 1:
// 		case 2:
// 		case 3:
// 		case 4:
// 		case 5:
// 		case 6:
// 		case 7:
// 		case 8:
// 		case 9:
// 			if (msTicks - owt_time > state->wait_time) {
// 				owt_state++;
// 				owt_time = msTicks;
// 				_command(config, state, (config->adc_mode << 7) | 0x268, msTicks);
// 			}
// 			return LTC6804_WAITING;
// 		case 10:
// 			if (msTicks - owt_time > state->wait_time) {
// 				uint8_t *normal_rx_ptr = state->rx_buf;
// 				state->rx_buf = owt_up_rx_buf[0];
// 				LTC6804_STATUS_T r;
// 				r = _read(config, state, RDCVA, msTicks);
// 				if (r != LTC6804_PASS) {state->rx_buf = normal_rx_ptr; return r;}
// 				for (i = 0; i < config->num_modules; i++) {
// 					uint8_t *rx_ptr = owt_up_rx_buf[0] + 4 + 8 * i;
// 					if (rx_ptr[0] == 0 && rx_ptr[1] == 0) {
// 						state->rx_buf = normal_rx_ptr;
// 						owt_state = 0;
// 						return LTC6804_FAIL; // [TODO] Also return module and cell 0
// 					} else {
// 						// Chip_UART_SendBlocking(LPC_USART, "C[0], ", 6);
// 					}
// 					if (rx_ptr[0] == 0xFF || rx_ptr[1] == 0xFF) {
// 						Chip_UART_SendBlocking(LPC_USART, "FUCK ", 5);
// 					}
// 				}
// 				state->rx_buf = owt_up_rx_buf[1];
// 				r = _read(config, state, RDCVB, msTicks);
// 				if (r != LTC6804_PASS) {state->rx_buf = normal_rx_ptr; return r;}
// 				state->rx_buf = owt_up_rx_buf[2];
// 				r = _read(config, state, RDCVC, msTicks);
// 				if (r != LTC6804_PASS) {state->rx_buf = normal_rx_ptr; return r;}
// 				state->rx_buf = owt_up_rx_buf[3];
// 				r = _read(config, state, RDCVD, msTicks);
// 				if (r != LTC6804_PASS) {state->rx_buf = normal_rx_ptr; return r;}
// 				state->rx_buf = normal_rx_ptr;
// 				owt_state++;
// 				owt_time = msTicks;
// 				_command(config, state, (config->adc_mode << 7) | 0x228, msTicks);
// 			}
// 			return LTC6804_WAITING;
// 		case 11:
// 		case 12:
// 		case 13:
// 		case 14:
// 		case 15:
// 		case 16:
// 		case 17:
// 		case 18:
// 		case 19:
// 			if (msTicks - owt_time > state->wait_time) {
// 				owt_state++;
// 				owt_time = msTicks;
// 				_command(config, state, (config->adc_mode << 7) | 0x228, msTicks);
// 			}
// 			return LTC6804_WAITING;
// 		case 20:
// 			if (msTicks - owt_time > state->wait_time) {
// 				LTC6804_STATUS_T r;
// 				uint8_t str[10];
// 				int32_t c_up, c_down;
// 				r = _read(config, state, RDCVA, msTicks);
// 				if (r != LTC6804_PASS) {return r;}
// 				for (i = 0; i < config->num_modules; i++) {
// 					uint8_t *rx_ptr = state->rx_buf + 4 + 8 * i;
// 					uint8_t *owt_ptr = owt_up_rx_buf[0] + 4 + 8 * i;
// 					c_up = (owt_ptr[3] << 8) | owt_ptr[2];
// 					c_down = (rx_ptr[3] << 8) | rx_ptr[2];
// 					itoa(rx_ptr[2], str, 16);
// 					//Chip_UART_SendBlocking(LPC_USART, str, strlen(str));
// 					//Chip_UART_SendBlocking(LPC_USART, " : ", 3);
// 					itoa(rx_ptr[3], str, 16);
// 					//Chip_UART_SendBlocking(LPC_USART, str, strlen(str));
// 					//Chip_UART_SendBlocking(LPC_USART, " : ", 3);
// 					itoa(owt_ptr[2], str, 16);
// 					//Chip_UART_SendBlocking(LPC_USART, str, strlen(str));
// 					//Chip_UART_SendBlocking(LPC_USART, " : ", 3);
// 					itoa(owt_ptr[3], str, 16);
// 					//Chip_UART_SendBlocking(LPC_USART, str, strlen(str));
// 					//Chip_UART_SendBlocking(LPC_USART, " : ", 3);
// 					itoa(c_up - c_down, str, 10);
// 					//Chip_UART_SendBlocking(LPC_USART, str, strlen(str));
// 					if (c_up -  c_down < -4000) {
// 						//Chip_UART_SendBlocking(LPC_USART, ", ", 2);
// 						owt_state = 0;
// 						return LTC6804_FAIL; // C[1] Fail
// 					} else {
// 						//Chip_UART_SendBlocking(LPC_USART, " C[1], ", 6);
// 					}
// 					c_up = (owt_ptr[5] << 8) | owt_ptr[4];
// 					c_down = (rx_ptr[5] << 8) | rx_ptr[4];
// 					itoa(rx_ptr[4], str, 16);
// //					Chip_UART_SendBlocking(LPC_USART, str, strlen(str));
// 					//Chip_UART_SendBlocking(LPC_USART, " : ", 3);
// 					itoa(rx_ptr[5], str, 16);
// 					//Chip_UART_SendBlocking(LPC_USART, str, strlen(str));
// 					//Chip_UART_SendBlocking(LPC_USART, " : ", 3);
// 					itoa(owt_ptr[4], str, 16);
// 					//Chip_UART_SendBlocking(LPC_USART, str, strlen(str));
// 					//Chip_UART_SendBlocking(LPC_USART, " : ", 3);
// 					itoa(owt_ptr[5], str, 16);
// 					//Chip_UART_SendBlocking(LPC_USART, str, strlen(str));
// 					//Chip_UART_SendBlocking(LPC_USART, " : ", 3);
// 					itoa(c_up - c_down, str, 10);
// 					//Chip_UART_SendBlocking(LPC_USART, str, strlen(str));
// 					if (c_up -  c_down < -4000) {
// 						//Chip_UART_SendBlocking(LPC_USART, ", ", 2);
// 						owt_state = 0;
// 						return LTC6804_FAIL; // C[2] Fail
// 					} else {
// 						//////Chip_UART_SendBlocking(LPC_USART, " C[2], ", 6);
// 					}
// 				}
// 				r = _read(config, state, RDCVB, msTicks);
// 				if (r != LTC6804_PASS) {return r;}
// 				for (i = 0; i < config->num_modules; i++) {
// 					uint8_t *rx_ptr = state->rx_buf + 4 + 8 * i;
// 					uint8_t *owt_ptr = owt_up_rx_buf[1] + 4 + 8 * i;
// 					c_up = (owt_ptr[1] << 8) | owt_ptr[0];
// 					c_down = (rx_ptr[1] << 8) | rx_ptr[0];
// 					itoa(c_up - c_down, str, 10);
// 					//////Chip_UART_SendBlocking(LPC_USART, str, strlen(str));
// 					if (c_up - c_down < -4000) {
// 						//////Chip_UART_SendBlocking(LPC_USART, ", ", 2);
// 						owt_state = 0;
// 						return LTC6804_FAIL; // C[3] Fail
// 					} else {
// 						//////Chip_UART_SendBlocking(LPC_USART, " C[3], ", 6);
// 					}
// 					c_up = (owt_ptr[3] << 8) | owt_ptr[2];
// 					c_down = (rx_ptr[3] << 8) | rx_ptr[2];
// 					itoa(c_up - c_down, str, 10);
// 					//////Chip_UART_SendBlocking(LPC_USART, str, strlen(str));
// 					if (c_up - c_down < -4000) {
// 						//////Chip_UART_SendBlocking(LPC_USART, ", ", 2);
// 						owt_state = 0;
// 						return LTC6804_FAIL; // C[4] Fail
// 					} else {
// 						//////Chip_UART_SendBlocking(LPC_USART, " C[4], ", 6);
// 					}
// 					c_up = (owt_ptr[5] << 8) | owt_ptr[4];
// 					c_down = (rx_ptr[5] << 8) | rx_ptr[4];
// 					itoa(c_up - c_down, str, 10);
// 					//////Chip_UART_SendBlocking(LPC_USART, str, strlen(str));
// 					if (c_up - c_down < -4000) {
// 						//////Chip_UART_SendBlocking(LPC_USART, ", ", 2);
// 						owt_state = 0;
// 						return LTC6804_FAIL; // C[5] Fail
// 					} else {
// 						//////Chip_UART_SendBlocking(LPC_USART, " C[5], ", 6);
// 					}
// 				}
// 				r = _read(config, state, RDCVC, msTicks);
// 				if (r != LTC6804_PASS) {return r;}
// 				for (i = 0; i < config->num_modules; i++) {
// 					uint8_t *rx_ptr = state->rx_buf + 4 + 8 * i;
// 					uint8_t *owt_ptr = owt_up_rx_buf[2] + 4 + 8 * i;
// 					c_up = (owt_ptr[1] << 8) | owt_ptr[0];
// 					c_down = (rx_ptr[1] << 8) | rx_ptr[0];
// 					itoa(c_up - c_down, str, 10);
// 					////Chip_UART_SendBlocking(LPC_USART, str, strlen(str));
// 					if (c_up -  c_down < -4000) {
// 						////Chip_UART_SendBlocking(LPC_USART, ", ", 2);
// 						owt_state = 0;
// 						return LTC6804_FAIL; // C[6] Fail
// 					} else {
// 						////Chip_UART_SendBlocking(LPC_USART, " C[6], ", 6);
// 					}
// 					c_up = (owt_ptr[3] << 8) | owt_ptr[2];
// 					c_down = (rx_ptr[3] << 8) | rx_ptr[2];
// 					itoa(c_up - c_down, str, 10);
// 					////Chip_UART_SendBlocking(LPC_USART, str, strlen(str));
// 					if (c_up -  c_down < -4000) {
// 						////Chip_UART_SendBlocking(LPC_USART, ", ", 2);
// 						owt_state = 0;
// 						return LTC6804_FAIL; // C[7] Fail
// 					} else {
// 						////Chip_UART_SendBlocking(LPC_USART, " C[7], ", 6);
// 					}
// 					c_up = (owt_ptr[5] << 8) | owt_ptr[4];
// 					c_down = (rx_ptr[5] << 8) | rx_ptr[4];
// 					////itoa(c_up - c_down, str, 10);
// 					Chip_UART_SendBlocking(LPC_USART, str, strlen(str));
// 					if (c_up - c_down < -4000) {
// 						////Chip_UART_SendBlocking(LPC_USART, ", ", 2);
// 						owt_state = 0;
// 						return LTC6804_FAIL; // C[8] Fail
// 					} else {
// 						////Chip_UART_SendBlocking(LPC_USART, " C[8], ", 6);
// 					}
// 				}
// 				r = _read(config, state, RDCVD, msTicks);
// 				if (r != LTC6804_PASS) {return r;}
// 				for (i = 0; i < config->num_modules; i++) {
// 					uint8_t *rx_ptr = state->rx_buf + 4 + 8 * i;
// 					uint8_t *owt_ptr = owt_up_rx_buf[3] + 4 + 8 * i;
// 					c_up = (owt_ptr[1] << 8) | owt_ptr[0];
// 					c_down = (rx_ptr[1] << 8) | rx_ptr[0];
// 					itoa(c_up - c_down, str, 10);
// 					//Chip_UART_SendBlocking(LPC_USART, str, strlen(str));
// 					if (c_up - c_down < -4000) {
// 						//Chip_UART_SendBlocking(LPC_USART, ", ", 2);
// 						owt_state = 0;
// 						return LTC6804_FAIL; // C[9] Fail
// 					}  else {
// 						//Chip_UART_SendBlocking(LPC_USART, " C[9], ", 6);
// 					}
// 					c_up = (owt_ptr[3] << 8) | owt_ptr[2];
// 					c_down = (rx_ptr[3] << 8) | rx_ptr[2];
// 					itoa(c_up - c_down, str, 10);
// 					//Chip_UART_SendBlocking(LPC_USART, str, strlen(str));
// 					if (c_up - c_down < -4000) {
// 						//Chip_UART_SendBlocking(LPC_USART, ", ", 2);
// 						owt_state = 0;
// 						return LTC6804_FAIL; // C[10] Fail
// 					} else {
// 						//Chip_UART_SendBlocking(LPC_USART, " C[10], ", 7);
// 					}
// 					c_up = (owt_ptr[5] << 8) | owt_ptr[4];
// 					c_down = (rx_ptr[5] << 8) | rx_ptr[4];
// 					itoa(c_up - c_down, str, 10);
// 					//Chip_UART_SendBlocking(LPC_USART, str, strlen(str));
// 					if (c_up - c_down < -4000) {
// 						//Chip_UART_SendBlocking(LPC_USART, ", ", 2);
// 						owt_state = 0;
// 						return LTC6804_FAIL; // C[11] Fail
// 					} else {
// 						//Chip_UART_SendBlocking(LPC_USART, " C[11], ", 7);
// 					}
// 					if (rx_ptr[5] == 0 && rx_ptr[4] == 0) {
// 						//Chip_UART_SendBlocking(LPC_USART, ", ", 2);
// 						owt_state = 0;
// 						return LTC6804_FAIL; // C[12] Fail
// 					} else {
// 						//Chip_UART_SendBlocking(LPC_USART, " C[12]", 5);
// 					}

// 				}
// 				owt_state = 0;
// 				owt_time = msTicks;
// 				return LTC6804_PASS;
// 			}
// 			return LTC6804_WAITING;
// 		default:
// 			return LTC6804_FAIL; // What happened here
// 	}
// }

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

// [TODO] Clear cell votlages and only return pass when recieving not all FF
LTC6804_STATUS_T LTC6804_GetCellVoltages(LTC6804_CONFIG_T *config, LTC6804_STATE_T *state, LTC6804_ADC_RES_T *res, uint32_t msTicks) {
	if (_IS_ASLEEP(state, msTicks)) {
		_wake(config, state, msTicks, false);
		return LTC6804_WAITING;
	} else if (!_IS_REFUP(state, msTicks)) {
		return LTC6804_WAITING_REFUP;
	}

	if (!state->waiting) { // Need to send out self test command
		state->waiting = true;
		state->flight_time = msTicks;
		// _command(config, state, (config->adc_mode << 7) | 0x270, msTicks); // ADCV, All Channels, DCP=1
		_command(config, state, (config->adc_mode << 7) | cmd, msTicks);
		return LTC6804_WAITING;
	} else {
		if (msTicks - state->flight_time > state->wait_time) { // We've waited long enough
			state->waiting = false;
			int i, j;
			uint32_t *vol_ptr = res->cell_voltages_mV;
			uint32_t min = UINT32_MAX;
			uint32_t max = 0;
			// Read a cell voltage group
				// For each module, calculate head of module voltage group
				// vol_ptr is pointer to head of module cell group voltages
			LTC6804_STATUS_T r;
			r = _read(config, state, RDCVA, msTicks);
			if (r == LTC6804_SPI_ERROR || r == LTC6804_PEC_ERROR) return r;
			for (i = 0; i < config->num_modules; i++) {
				uint8_t *rx_ptr = state->rx_buf + 4 + 8 * i;
				for (j = 0; j < config->module_cell_count[i] && j < 3; j++) {
					vol_ptr[j] = (rx_ptr[2*j + 1] << 8 | rx_ptr[2*j]) / 10;
					max = (vol_ptr[j] > max) ? vol_ptr[j] : max;
					min = (vol_ptr[j] < min) ? vol_ptr[j] : min;
				}
				vol_ptr = vol_ptr + config->module_cell_count[i];
			}

			r = _read(config, state, RDCVB, msTicks);
			if (r == LTC6804_SPI_ERROR || r == LTC6804_PEC_ERROR) return r;
			vol_ptr = res->cell_voltages_mV;
			for (i = 0; i < config->num_modules; i++) {
				uint8_t *rx_ptr = state->rx_buf + 4 + 8 * i;
				for (j = 0; j + 3 < config->module_cell_count[i] && j < 3; j++) {
					vol_ptr[j + 3] = (rx_ptr[2 * j + 1] << 8 | rx_ptr[2* j]) / 10;
					max = (vol_ptr[j + 3] > max) ? vol_ptr[j + 3] : max;
					min = (vol_ptr[j + 3] < min) ? vol_ptr[j + 3] : min;
				} 
				vol_ptr = vol_ptr + config->module_cell_count[i];
			}

			r = _read(config, state, RDCVC, msTicks);
			if (r == LTC6804_SPI_ERROR || r == LTC6804_PEC_ERROR) return r;
			vol_ptr = res->cell_voltages_mV;
			for (i = 0; i < config->num_modules; i++) {
				uint8_t *rx_ptr = state->rx_buf + 4 + 8 * i;
				for (j = 0; j + 6 < config->module_cell_count[i] && j < 3; j++) {
					vol_ptr[j + 6] = (rx_ptr[2 * j + 1] << 8 | rx_ptr[2 * j]) / 10;
					max = (vol_ptr[j + 6] > max) ? vol_ptr[j + 6] : max;
					min = (vol_ptr[j + 6] < min) ? vol_ptr[j + 6] : min;
				} 
				vol_ptr = vol_ptr + config->module_cell_count[i];
			}
			
			r = _read(config, state, RDCVD, msTicks);
			if (r == LTC6804_SPI_ERROR || r == LTC6804_PEC_ERROR) return r;
			vol_ptr = res->cell_voltages_mV;
			for (i = 0; i < config->num_modules; i++) {
				uint8_t *rx_ptr = state->rx_buf + 4 + 8 * i;
				for (j = 0; j + 9 < config->module_cell_count[i] && j < 3; j++) {
					vol_ptr[j + 9] = (rx_ptr[2 * j + 1] << 8 | rx_ptr[2 * j]) / 10;
					max = (vol_ptr[j + 9] > max) ? vol_ptr[j + 9] : max;
					min = (vol_ptr[j + 9] < min) ? vol_ptr[j + 9] : min;
				} 
				vol_ptr = vol_ptr + config->module_cell_count[i];
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
	uint32_t res = Chip_SSP_WriteFrames_Blocking(config->pSSP, state->tx_buf, 4);
	Chip_GPIO_SetPinState(LPC_GPIO, config->cs_gpio, config->cs_pin, true);

	if (res == ERROR) return LTC6804_SPI_ERROR;
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
	uint32_t res = Chip_SSP_WriteFrames_Blocking(config->pSSP, state->tx_buf, 4+8*config->num_modules);
	Chip_GPIO_SetPinState(LPC_GPIO, config->cs_gpio, config->cs_pin, true);

	if (res == ERROR) return LTC6804_SPI_ERROR;
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
	uint32_t res = Chip_SSP_RWFrames_Blocking(config->pSSP, state->xf);
	Chip_GPIO_SetPinState(LPC_GPIO, config->cs_gpio, config->cs_pin, true);

	if (res == ERROR) return LTC6804_SPI_ERROR;

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
	uint32_t res = Chip_SSP_WriteFrames_Blocking(config->pSSP, wake_buf, wake);
	Chip_GPIO_SetPinState(LPC_GPIO, config->cs_gpio, config->cs_pin, true);

	// If you fall asleep, write configuration again pls
	if (waking) LTC6804_WriteCFG(config, state, msTicks); // Wake will be skipped bc last_message updated
	if (waking && state->balancing) _set_balance_states(config, state, msTicks);

	if (res == ERROR) return LTC6804_SPI_ERROR;
	return LTC6804_PASS;
}

LTC6804_STATUS_T _set_balance_states(LTC6804_CONFIG_T *config, LTC6804_STATE_T *state, uint32_t msTicks) {
	int i;
	for (i = 0; i < config->num_modules; i++) {
		uint8_t *tx_ptr = state->tx_buf + 4 + 8 * i;
		tx_ptr[0] = state->cfg[0]; tx_ptr[1] = state->cfg[1];
		tx_ptr[2] = state->cfg[2]; tx_ptr[3] = state->cfg[3];
		tx_ptr[4] = state->bal_list[i] & 0xFF;
		tx_ptr[5] = (state->cfg[5] & 0xF0) | (state->bal_list[i] >> 8);
		uint16_t pec = _calculate_pec(tx_ptr, 6);
		tx_ptr[6] = pec >> 8;
		tx_ptr[7] = pec & 0xFF;
	}
	return _write(config, state, WRCFG, msTicks);
}

uint16_t _calculate_pec(uint8_t *data, uint8_t len) {

	int i,j;
	uint16_t pec = 0x0010;

	for (i = 0; i < len; i++) {
		for (j = 0; j < 8; j++) {
			uint16_t din = data[i] >> (7-j) & 1;
			uint16_t in0 = din ^ (pec >> 14 & 1); 				//in0 = din ^ pec[14]
			uint16_t in3 = ((in0 << 2) ^ (pec & 0x0004)) << 1;	//in3 = in0 ^ pec[2]
			uint16_t in4 = ((in0 << 3) ^ (pec & 0x0008)) << 1;	//in4 = in0 ^ pec[3]
			uint16_t in7 = ((in0 << 6) ^ (pec & 0x0040)) << 1;	//in7 = in0 & pec[6]
			uint16_t in8 = ((in0 << 7) ^ (pec & 0x0080)) << 1;	//in8 = in0 & pec[7];
			uint16_t in10 = ((in0 << 9) ^ (pec & 0x0200)) << 1;	//in10 = in0 ^ pec[9]
			uint16_t in14 = ((in0 << 13) ^ (pec & 0x2000)) << 1; //in14 = in0 ^ pec[13];
			
			pec = (pec & ~0x4000) + in14;					//pec14 = in14
			pec = (pec & ~0x2000) + ((pec & 0x1000) << 1);	//pec13 = pec12
			pec = (pec & ~0x1000) + ((pec & 0x0800) << 1);	//pec12 = pec11
			pec = (pec & ~0x0800) + ((pec & 0x0400) << 1);	//pec11 = pec10
			pec = (pec & ~0x0400) + in10;					//pec10 = in10
			pec = (pec & ~0x0200) + ((pec & 0x0100) << 1);	//pec9 = pec8
			pec = (pec & ~0x0100) + in8;					//pec8 = in8
			pec = (pec & ~0x0080) + in7;					//pec7 = in7
			pec = (pec & ~0x0040) + ((pec & 0x0020) << 1);	//pec6 = pec5
			pec = (pec & ~0x0020) + ((pec & 0x0010) << 1);	//pec5 = pec4
			pec = (pec & ~0x0010) + in4;					//pec4 = in4;
			pec = (pec & ~0x0008) + in3; 					//pec3 = in3
			pec = (pec & ~0x0004) + ((pec & 0x0002) << 1); 	//pec2 = pec1
			pec = (pec & ~0x0002) + ((pec & 0x0001) << 1); 	//pec1 = pec0
			pec = (pec & ~0x0001) + in0; 					//pec0 = in0
		}
		
	}

	return pec << 1;
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
