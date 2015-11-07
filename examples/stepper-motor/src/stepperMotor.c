#include "chip.h"
#include "stepperMotor.h"

void Stepper_StepCases(STEPPER_MOTOR_T *mot, int32_t step){
	switch (step){
		case 3:
			Chip_GPIO_SetPinState(LPC_GPIO, mot->ports[0], mot->pins[0], true);
			Chip_GPIO_SetPinState(LPC_GPIO, mot->ports[1], mot->pins[1], false);
			Chip_GPIO_SetPinState(LPC_GPIO, mot->ports[2], mot->pins[2], true);
			Chip_GPIO_SetPinState(LPC_GPIO, mot->ports[3], mot->pins[3], false);
			break;
		case 2:
			Chip_GPIO_SetPinState(LPC_GPIO, mot->ports[0], mot->pins[0], false);
			Chip_GPIO_SetPinState(LPC_GPIO, mot->ports[1], mot->pins[1], true);
			Chip_GPIO_SetPinState(LPC_GPIO, mot->ports[2], mot->pins[2], true);
			Chip_GPIO_SetPinState(LPC_GPIO, mot->ports[3], mot->pins[3], false);
			break;
		case 1:
			Chip_GPIO_SetPinState(LPC_GPIO, mot->ports[0], mot->pins[0], false);
			Chip_GPIO_SetPinState(LPC_GPIO, mot->ports[1], mot->pins[1], true);
			Chip_GPIO_SetPinState(LPC_GPIO, mot->ports[2], mot->pins[2], false);
			Chip_GPIO_SetPinState(LPC_GPIO, mot->ports[3], mot->pins[3], true);
			break;
		case 0:
			Chip_GPIO_SetPinState(LPC_GPIO, mot->ports[0], mot->pins[0], true);
			Chip_GPIO_SetPinState(LPC_GPIO, mot->ports[1], mot->pins[1], false);
			Chip_GPIO_SetPinState(LPC_GPIO, mot->ports[2], mot->pins[2], false);
			Chip_GPIO_SetPinState(LPC_GPIO, mot->ports[3], mot->pins[3], true);
			break;
	}
}

void Stepper_Init(STEPPER_MOTOR_T *mot){

	//Initializes pins
	Chip_GPIO_Init(LPC_GPIO);
	Chip_GPIO_WriteDirBit(LPC_GPIO, mot->ports[0], mot->pins[0], true);
	Chip_GPIO_WriteDirBit(LPC_GPIO, mot->ports[1], mot->pins[1], true);
	Chip_GPIO_WriteDirBit(LPC_GPIO, mot->ports[2], mot->pins[2], true);
	Chip_GPIO_WriteDirBit(LPC_GPIO, mot->ports[3], mot->pins[3], true);
	
	mot->zeroing = false;
	mot->pos = 0;
	mot->ticks = 0;
	mot->step_num = 0;

}

void Stepper_ChoosePosition(STEPPER_MOTOR_T *mot, uint8_t percent, volatile uint32_t msTicks){
	int32_t turn = (percent * mot->step_per_rotation) / 100;
	Stepper_Spin(mot, turn - mot->pos, msTicks);
}

void Stepper_ZeroPosition(STEPPER_MOTOR_T *mot, volatile uint32_t msTicks){
	mot->new_pos = mot->pos - mot->step_per_rotation;
	mot->zeroing = true;
	mot->ticks = msTicks;
}

void Stepper_HomePosition(STEPPER_MOTOR_T *mot, volatile uint32_t msTicks){
	Stepper_Spin(mot, mot->pos * -1, msTicks);
}

void Stepper_Spin(STEPPER_MOTOR_T *mot, int32_t steps, volatile uint32_t msTicks){
	mot->new_pos = steps - mot->pos;
	mot->ticks =  msTicks; 
}

void Stepper_Step(STEPPER_MOTOR_T *mot, volatile uint32_t msTicks){
	if (mot->new_pos != mot->pos){
		if (msTicks - mot->ticks >= mot->step_delay){
			mot->ticks = msTicks;
			if (mot->new_pos > mot->pos){
				mot->step_num++;
				mot->pos++;
				Stepper_StepCases(mot, mot->step_num%4);	
			} else {
				mot->step_num--;
				mot->pos--;
				Stepper_StepCases(mot, mot->step_num%4);	
			}
			if (mot->zeroing && mot->pos == mot->new_pos){
				mot->zeroing = false;
				mot->pos = 0;
				mot->new_pos = 0;
			}
		} 
	}
	
}




//TEST CODE BELOW ---------------------------------------

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
		
// 		Stepper_ChoosePosition(75);
// 		Delay(100);
		
// 		Stepper_Step(320);
// 		Delay(100);	
		
// 		Stepper_Step(80);
// 		Delay(100);

// 		Stepper_Step(160)
// 		Stepper_ZeroPosition();
// 	}



// }


