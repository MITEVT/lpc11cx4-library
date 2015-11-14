#include "chip.h"
#include "board.h"
#include "util.h"

volatile uint32_t msTicks;

int main(void){
	if (Board_SysTick_Init()) {
		while(1);
	}
	
	Board_DAC_Init();
//	Board_DAC_SetVoltage(1650);

	Board_UART_Init(9600);
	Board_UART_Println("Started");	

	uint16_t num = 1500;
	int16_t delta = 5;
	uint32_t last = msTicks;

	

	while(1){
		if(last+200<msTicks){
//			if(num+delta>=100 && num+delta <= 3200)
//				num+=delta;
//			else delta = -delta;
			if(num==825){
				num=2475;
			}
			else num = 825;
	                Board_DAC_SetVoltage(num);
			last = msTicks;
                }
	}
}
