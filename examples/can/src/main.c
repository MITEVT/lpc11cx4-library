#include "chip.h"

const uint32_t OscRateIn = 12000000;

int main(void)
{

	SystemCoreClockUpdate();

	uint32_t CanApiClkInitTable[2] = { 0x00000000UL, // CANCLKDIV 
									   0x00004DC5UL // CANBT
	}; 

	// Second Paramter says no interrupts
	LPC_CCAN_API->init_can(&CanApiClkInitTable[0], 0);

	CCAN_MSG_OBJ_T msg_obj;

	//Set message object 1 to recieve all 11-bit messages 0x300-0x3FF
	msg_obj.msgobj = 1;
	msg_obj.mode_id = 0x300;
	msg_obj.mask = 0x700;
	LPC_CCAN_API->config_rxmsgobj(&msg_obj);

	//Set message object 2 to receive all 11-bit message 0x200-0x2F0
	msg_obj.msgobj = 2;
	msg_obj.mode_id = 0x200;
	msg_obj.mask = 0x70F;
	LPC_CCAN_API->config_rxmsgobj(&msg_obj);

	while (1) {
		msg_obj.msgobj = 1;
		LPC_CCAN_API->can_receive(&msg_obj);

		if (msg_obj.mode_id == 0x200) {
			//Received 0x200 Message
			msg_obj.msgobj = 3;
			msg_obj.mode_id = 0x020;
			msg_obj.mask = 0x0;
			msg_obj.dlc = 1;
			msg_obj.data[0] = 0x00; 
			LPC_CCAN_API->can_transmit(&msg_obj);
		} else if (msg_obj.mode_id == 0x300) {
			//Received 0x300 Message
			msg_obj.msgobj = 4;
			msg_obj.mode_id = 0x020;
			msg_obj.mask = 0x0;
			msg_obj.dlc = 1;
			msg_obj.data[0] = 0x00; 
			LPC_CCAN_API->can_transmit(&msg_obj);
		}
	}

	return 0;
}
