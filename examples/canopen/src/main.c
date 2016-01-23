#include "board.h"

// -------------------------------------------------------------
// Macro Definitions

#define CCAN_BAUD_RATE 500000 					// Desired CAN Baud Rate
#define UART_BAUD_RATE 57600 					// Desired UART Baud Rate

#define BUFFER_SIZE 8
#define NODE_ID 2						// The ID for CANopen, 1 for the server, 2 for the client

// -------------------------------------------------------------
// Static Variable Declaration

extern volatile uint32_t msTicks;

static CCAN_MSG_OBJ_T msg_obj; 					// Message Object data structure for manipulating CAN messages
static RINGBUFF_T can_rx_buffer;				// Ring Buffer for storing received CAN messages
static CCAN_MSG_OBJ_T _rx_buffer[BUFFER_SIZE]; 	// Underlying array used in ring buffer

static uint8_t uart_rx_buffer[BUFFER_SIZE]; 	// UART received message buffer

static uint8_t testval;		// The value stored for the CANopen SDO, Tested for controlling an LED on the devboard

// -------------------------------------------------------------
// Helper Functions

/**
 * Delay the processor for a given number of milliseconds
 * @param ms Number of milliseconds to delay
 */
void _delay(uint32_t ms) {
	uint32_t curTicks = msTicks;
	while ((msTicks - curTicks) < ms);
}
// -------------------------------------------------------------
// Interrupt Service Routines


// -------------------------------------------------------------
// Main Program Loop

int main(void)
{

	testval = 0;

	//---------------
	// Initialize SysTick Timer to generate millisecond count
	if (Board_SysTick_Init()) {
		// Unrecoverable Error. Hang.
		while(1);
	}

	//---------------
	// Initialize GPIO and LED as output
	Board_LEDs_Init();

	//---------------
	// Initialize UART Communication
	Board_UART_Init(UART_BAUD_RATE);
	Board_UART_Println("Started up");

	//---------------
	// Initialize CAN  and CAN Ring Buffer
	//
	CCAN_CANOPENCFG_T canopen_cfg;
	canopen_cfg.node_id=NODE_ID;			// The ID for the node, set to 1 for the server, anything else for the client
	canopen_cfg.msgobj_rx=1;
	canopen_cfg.msgobj_tx=2;
	canopen_cfg.isr_handled=true;		// Automatically handle CANopen requests in an interrupt
	canopen_cfg.od_const_num=1;		// There is one entry in the constant value table
	CCAN_ODCONSTENTRY_T od_const_table[1];
	od_const_table[0].index=0x1000;		// Tier 1 of the identification
	od_const_table[0].subindex=0x00;	// Tier 2 of the identification in the object dictionary
	od_const_table[0].len=1;		
	od_const_table[0].val=0x42;		// The value being stored here
	canopen_cfg.od_const_table=(CCAN_ODCONSTENTRY_T *)od_const_table;	// VERY IMPORTANT: Remember to actually put the table in the structure. 
										// You may laugh, but this comment will save someone a huge headache and a couple of hours
	canopen_cfg.od_num=1;			// There is only 1 value in the object dictionary
	CCAN_ODENTRY_T od_table[1];
	od_table[0].index=0x2000;		// Tier 1 of the identification
	od_table[0].subindex=0x00;		// Tier 2 of the identification
	od_table[0].entrytype_len=OD_EXP_RW | 1;// It is only a length of 1, and is used for expedited RW
	od_table[0].val=&(testval);		// Assign the pointer to the object dictionary
	canopen_cfg.od_table =(CCAN_ODENTRY_T *) od_table;			// VERY IMPORTANT: Remember to actually put the table in the structure.
										// You may laugh, but this comment will save someone a huge headache and a couple of hours

	RingBuffer_Init(&can_rx_buffer, _rx_buffer, sizeof(CCAN_MSG_OBJ_T), BUFFER_SIZE);
	RingBuffer_Flush(&can_rx_buffer);

	Board_CAN_Init(CCAN_BAUD_RATE, &canopen_cfg);

	testval = 1;

	uint32_t lastChange = msTicks;
	bool on = true;
	uint8_t sdo_responses[8];
	Board_LEDtwo_On();		// Turn on the blinking LED
	while (1) {
	if(lastChange+200<msTicks){	//Blink the LED
		if(on)
			Board_LEDtwo_Off();
		else
			Board_LEDtwo_On();
		on = !on;
		lastChange = msTicks;
	}
	if(testval>0)			// Test the value in the CANopen Object dictionary, and toggle the LED
		Board_LEDs_On();
	else
		Board_LEDs_Off();		
	uint8_t count;
	uint8_t val;
	if(Board_CANopen_NewData()){			// Check for any new CANopen data
		Board_UART_Println("Recieved Data:");	
		int i=0;
		for(i=0;i<8;i++){			// Print all of the data recieved, 0-3 are id information, 4-7 are the response data such as the requested value
			Board_UART_Print("Value: 0x");
			Board_UART_PrintNum(sdo_responses[i],16,true);
		}
	}
	if ((count = Board_UART_Read(uart_rx_buffer, BUFFER_SIZE)) != 0) {
			switch (uart_rx_buffer[0]) {
				case 'a':
					Board_UART_Println("Enabling the LED");
					val = 0xFF;
					Board_CANopen_Write(1,0x2000,0x00,&val,1);	// Write a high value to the server's testvalue to enable the LED
					testval = val;
					break;
				case 'b':
					Board_UART_Println("Disabling the LED");
					val = 0x00;
					Board_CANopen_Write(1,0x2000,0x00,&val,1);	// Write a 0 to the server's testvalue to disable the LED
					testval = val;
					break;
				case 'c':
					Board_CANopen_Read(1, 0x2000, 0x00, sdo_responses);	// Read the server's testvalue, should print 0xFF or 0x0 depending on if the LED is on or off
					break;
				case 'd':
					Board_CANopen_Read(1,0x1000,0x00, sdo_responses);	// Read the server's constant value, should print 0x42
					break;
				default:
					Board_UART_Println("Invalid Command");
					break;
			}
		}
	}
}
