#include "board.h"

// -------------------------------------------------------------
// Macro Definitions

#define CCAN_BAUD_RATE 500000 					// Desired CAN Baud Rate
#define UART_BAUD_RATE 57600 					// Desired UART Baud Rate

#define BUFFER_SIZE 8

// -------------------------------------------------------------
// Static Variable Declaration

extern volatile uint32_t msTicks;

static CCAN_MSG_OBJ_T msg_obj; 					// Message Object data structure for manipulating CAN messages
static RINGBUFF_T can_rx_buffer;				// Ring Buffer for storing received CAN messages
static CCAN_MSG_OBJ_T _rx_buffer[BUFFER_SIZE]; 	// Underlying array used in ring buffer

static char str[100];							// Used for composing UART messages
static uint8_t uart_rx_buffer[BUFFER_SIZE]; 	// UART received message buffer

static bool can_error_flag;
static uint32_t can_error_info;

static uint8_t testval;

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
// CAN Driver Callback Functions

/*	CAN receive callback */
/*	Function is executed by the Callback handler after
    a CAN message has been received */
void CAN_rx(uint8_t msg_obj_num) {
	Board_LEDs_Off();
	CCAN_MSG_OBJ_T CANopen_Msg_Obj;

         /* Determine which CAN message has been received */
	CANopen_Msg_Obj.msgobj = msg_obj_num;
 
	 /* Now load up the CANopen_Msg_Obj structure with the CAN message */
	LPC_CCAN_API->can_receive(&CANopen_Msg_Obj);
}

/*	CAN transmit callback */
/*	Function is executed by the Callback handler after
    a CAN message has been transmitted */
void CAN_tx(uint8_t msg_obj_num) {
}

/*	CAN error callback */
/*	Function is executed by the Callback handler after
    an error has occurred on the CAN bus */
void CAN_error(uint32_t error_info) {
	can_error_info = error_info;
	can_error_flag = true;
}

uint32_t CANOPEN_sdo_read(uint16_t index, uint8_t subindex){
	return 0;
}

uint32_t CANOPEN_sdo_write(uint16_t index, uint8_t subindex, uint8_t *dat_ptr){
	return 0;
}

uint8_t CANOPEN_sdo_req(uint8_t length, uint8_t *req_ptr, uint8_t *length_resp, uint8_t *resp_ptr){
	return CAN_SDOREQ_NOTHANDLED;
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
	canopen_cfg.node_id=1;
	canopen_cfg.msgobj_rx=1;
	canopen_cfg.msgobj_tx=2;
	canopen_cfg.isr_handled=true;
	canopen_cfg.od_const_num=1;
	CCAN_ODCONSTENTRY_T od_const_table[1];
	od_const_table[0].index=0x1000;
	od_const_table[0].subindex=0x00;
	od_const_table[0].len=1;
	od_const_table[0].val=42;
	canopen_cfg.od_const_table=(CCAN_ODCONSTENTRY_T *)od_const_table;
	canopen_cfg.od_num=1;
	CCAN_ODENTRY_T od_table[1];
	od_table[0].index=0x2000;
	od_table[0].subindex=0x00;
	od_table[0].entrytype_len=OD_EXP_RW | 1;
	od_table[0].val=&(testval);
	canopen_cfg.od_table =(CCAN_ODENTRY_T *) od_table;
	RingBuffer_Init(&can_rx_buffer, _rx_buffer, sizeof(CCAN_MSG_OBJ_T), BUFFER_SIZE);
	RingBuffer_Flush(&can_rx_buffer);

	Board_CAN_Init(CCAN_BAUD_RATE, CAN_rx, CAN_tx, CAN_error, CANOPEN_sdo_read, CANOPEN_sdo_write, CANOPEN_sdo_req, &canopen_cfg);

	can_error_flag = false;
	can_error_info = 0;

	testval = 1;

	uint32_t lastChange = msTicks;
	bool on = true;
	Board_LEDtwo_On();
	while (1) {
	if(lastChange+200<msTicks){
		if(on)
			Board_LEDtwo_Off();
		else
			Board_LEDtwo_On();
		on = !on;
		lastChange = msTicks;
	}
	if(testval>0)
		Board_LEDs_On();
	else
		Board_LEDs_Off();		
	if(can_error_flag){
		Chip_GPIO_SetPinState(LPC_GPIO, 0, 7, false);
		can_error_flag = false;
		Board_UART_Print("CAN Error: 0b");
		itoa(can_error_info, str, 2);
		Board_UART_Println(str);
	}
	
	uint8_t count;
	uint8_t val;
	if ((count = Board_UART_Read(uart_rx_buffer, BUFFER_SIZE)) != 0) {
		Board_UART_SendBlocking(uart_rx_buffer, count); // Echo user input
			switch (uart_rx_buffer[0]) {
				case 'a':
					Board_UART_Println("Enabling the LED");
					val = 0xFF;
					Board_CANopen_Write(1,0x2000,0x00,&val,1);
					testval = val;
					break;
				case 'b':
					Board_UART_Println("Disabling the LED");
					val = 0x00;
					Board_CANopen_Write(1,0x2000,0x00,&val,1);
					testval = val;
					break;
				case 'c':
					Board_UART_Print("Value: 0b");
					Board_UART_PrintNum(testval, 2, true);
					break;
				default:
					Board_UART_Println("Invalid Command");
					break;
			}
		}
	}
}
