#include "chip.h"

//--------------------------------------------
// The stepper motors used in 2015-2016 (VID29-02P D1437CDB1) do not make
// full revolutions. They cover ~ 315 degrees, or 640 ticks
//
// STEPPER_INIT SHOULD HAVE 640 AS AN INPUT FOR THESE
//--------------------------------------------

typedef struct _STEPPER_MOTOR_T_{
	uint8_t ports[4];
	uint8_t pins[4];
	int32_t pos;
	int32_t new_pos;
	bool zeroing;
	uint32_t ticks;
	int32_t step_num;
	int32_t step_per_rotation;
	uint32_t step_delay;
} STEPPER_MOTOR_T;

typedef enum _STEPPER_MOTOR_STATE_T_ {
	STOPPED, MOVING, ZEROING
} STEPPER_STATE_T;

/**
 * @brief	The different input sequences that step the motor
 * @param	Integer, 0-3, representing the step 
 * @return	Nothing
 */
void Stepper_StepCases(STEPPER_MOTOR_T*, int32_t step);


/**
 * @brief	Initialize a stepper 
 * @param	Integer, number of steps in a full rotation
 * @return	Nothing
 */
void Stepper_Init(STEPPER_MOTOR_T*);

/**
 * @brief	Moves stepper to inputted location
 * @param	Integer, percentage you want stepper to cover
 * @return	Nothing
 */
void Stepper_SetPosition(STEPPER_MOTOR_T*, uint8_t percent, volatile uint32_t msTicks);

/**
 * @brief	Initially gets timer to 0 position
 * @param	None
 * @return	Nothing
 */
void Stepper_ZeroPosition(STEPPER_MOTOR_T*, volatile uint32_t msTicks);

/**
 * @brief	Gets timer to 0 position after initialization
 * @param	None
 * @return	Nothing
 */
void Stepper_HomePosition(STEPPER_MOTOR_T*, volatile uint32_t msTicks);

STEPPER_STATE_T Stepper_Spin(STEPPER_MOTOR_T*, int32_t steps, volatile uint32_t msTicks);

/**
 * @brief	Moves timer a certain number of step forward/backwards
 * @param	Steps to move; forward if positive, backwards if negative
 * @return	Nothing
 */
STEPPER_STATE_T Stepper_Step(STEPPER_MOTOR_T*, volatile uint32_t msTicks);


