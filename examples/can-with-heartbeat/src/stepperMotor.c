#include "chip.h"
#include "stdlib.h"
#include "stepperMotor.h"

static int64_t  stepperPosition = 0;          // The current position (in steps) relative to Zero
static int32_t stepperStepNumber = 0;        // The current position (in steps) relative to last stop
static uint32_t stepperStepsPerRotation = 0;  // Number of steps in a full rotation
static uint32_t stepperStepDelay = 0;         // Delay value based on speed


// const uint32_t OscRateIn = 12000000;

volatile uint32_t msTicks;



// void SysTick_Handler(void) {
// 	msTicks++;
// }

__INLINE static void Delay(uint32_t dlyTicks) {
	uint32_t curTicks = msTicks;
	while ((msTicks - curTicks) < dlyTicks);
}


void Stepper_StepCases(int step){
	switch (step){
		case 3:
			Chip_GPIO_SetPinState(LPC_GPIO, port, pin0, true);
			Chip_GPIO_SetPinState(LPC_GPIO, port, pin1, false);
			Chip_GPIO_SetPinState(LPC_GPIO, port, pin2, true);
			Chip_GPIO_SetPinState(LPC_GPIO, port, pin3, false);
			break;
		case 2:
			Chip_GPIO_SetPinState(LPC_GPIO, port, pin0, false);
			Chip_GPIO_SetPinState(LPC_GPIO, port, pin1, true);
			Chip_GPIO_SetPinState(LPC_GPIO, port, pin2, true);
			Chip_GPIO_SetPinState(LPC_GPIO, port, pin3, false);
			break;
		case 1:
			Chip_GPIO_SetPinState(LPC_GPIO, port, pin0, false);
			Chip_GPIO_SetPinState(LPC_GPIO, port, pin1, true);
			Chip_GPIO_SetPinState(LPC_GPIO, port, pin2, false);
			Chip_GPIO_SetPinState(LPC_GPIO, port, pin3, true);
			break;
		case 0:
			Chip_GPIO_SetPinState(LPC_GPIO, port, pin0, true);
			Chip_GPIO_SetPinState(LPC_GPIO, port, pin1, false);
			Chip_GPIO_SetPinState(LPC_GPIO, port, pin2, false);
			Chip_GPIO_SetPinState(LPC_GPIO, port, pin3, true);
			break;
	}
}

void Stepper_Init(int steps){

	//Initializes pins
	Chip_GPIO_Init(LPC_GPIO);
	Chip_GPIO_WriteDirBit(LPC_GPIO, port, pin0, true);
	Chip_GPIO_WriteDirBit(LPC_GPIO, port, pin1, true);
	Chip_GPIO_WriteDirBit(LPC_GPIO, port, pin2, true);
	Chip_GPIO_WriteDirBit(LPC_GPIO, port, pin3, true);

	//Setting defult values
	stepperStepsPerRotation = steps;
	Stepper_SetSpeed(90);


}

int Stepper_Position(void){
	return stepperPosition;
}

int Stepper_StepNumber(void){
	return stepperStepNumber;
}

int Stepper_TotalTicks(void){
	return stepperStepsPerRotation;
}



void Stepper_ChoosePosition(int percent){
	float turn = (percent / 100.0) * stepperStepsPerRotation;
	Stepper_Step(turn - Stepper_Position());
}

void Stepper_ZeroPosition(void){
	int i = stepperStepsPerRotation;
	while(i > 0){
		Stepper_StepCases(i % 4);
		Delay(8);
		i --;
	}
	stepperPosition = 0;
}

void Stepper_HomePosition(void){
	int i = stepperStepsPerRotation;
	while(i > 0){
		Stepper_StepCases(i % 4);
		Delay(8);
		i --;
	}
	stepperPosition = 0;
	//Stepper_Step(stepperPosition * -1);
}


void Stepper_SetSpeed(int rpm){
	float convert = 300.0/(3.2*rpm);
	stepperStepDelay = convert;
}

void Stepper_Step(int steps){
	int stepsTaken = 0;
	while (stepsTaken < abs(steps) && stepperPosition <= stepperStepsPerRotation && stepperPosition >= 0){
		Delay(stepperStepDelay);
		if (steps > 0){
			stepperStepNumber++;
			stepperPosition++;
			Stepper_StepCases(stepperStepNumber%4);
		} else {
			stepperStepNumber--;
			stepperPosition--;
			Stepper_StepCases(stepperStepNumber%4);		
		}
		stepsTaken++;
	}

}


// //TEST CODE BELOW ---------------------------------------

// int main(void){

// 	SystemCoreClockUpdate();


// 	if (SysTick_Config (SystemCoreClock / 1000)) {
// 		//Error
// 		while(1);
// 	}


// 	Stepper_Init(640);
// 	Stepper_ZeroPosition();
// 	Stepper_SetSpeed(46);
// 	Delay(150);

	
// 	while(1){
// 		Stepper_Step(640);
// 		Delay(100);
// 		Stepper_HomePosition();
// 		Delay(200);	
// 		Stepper_Step(60);
// 		Delay(100);	
// 		Stepper_Step(100);
// 		Delay(100);
// 		Stepper_HomePosition();
// 	}



// }


