#include "chip.h"



#define port 2
#define pin0 2 //PINOUT 26
#define pin1 3 //PINOUT 38
#define pin2 7 //PINOUT 11
#define pin3 8 //PINOUT 12


//--------------------------------------------
// The stepper motors used in 2015-2016 (VID29-02P D1437CDB1) do not make
// full revolutions. They cover ~ 315 degrees, or 640 ticks
//
// STEPPER_INIT SHOULD HAVE 640 AS AN INPUT FOR THESE
//--------------------------------------------


/**
 * @brief	The different input sequences that step the motor
 * @param	Integer, 0-3, representing the step 
 * @return	Nothing
 */
void Stepper_StepCases(int step);


/**
 * @brief	Initialize a stepper 
 * @param	Integer, number of steps in a full rotation
 * @return	Nothing
 */
void Stepper_Init(int steps);

/**
 * @brief	Gets stepper's position relative to zero
 * @param	None
 * @return	Integer, position from zero
 */
int Stepper_Position(void);

/**
 * @brief	Gets stepper's position relative to last stop
 * @param	None
 * @return	Integer, position from last stop
 */
int Stepper_StepNumber(void);

/**
 * @brief	Gets stepper's total ticks per full rotation
 * @param	None
 * @return	Integer, ticks per rotation
 */
int Stepper_TotalTicks(void);

/**
 * @brief	Moves stepper to inputted location
 * @param	Integer, percentage you want stepper to cover
 * @return	Nothing
 */
void Stepper_ChoosePosition(int percent);

/**
 * @brief	Initially gets timer to 0 position
 * @param	None
 * @return	Nothing
 */
void Stepper_ZeroPosition(void);

/**
 * @brief	Gets timer to 0 position after initialization
 * @param	None
 * @return	Nothing
 */
void Stepper_HomePosition(void);

/**
 * @brief	Sets delay so timer goes at certain speed in revolutions per minute
 * @param	Integer, speed in rpm up to 46
 * @return	Nothing
 */
void Stepper_SetSpeed(int speed);

/**
 * @brief	Moves timer a certain number of step forward/backwards
 * @param	Steps to move; forward if positive, backwards if negative
 * @return	Nothing
 */
void Stepper_Step(int steps);



