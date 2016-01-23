#include "board.h"
#define ID 1

// -------------------------------------------------------------
// Board ISRs

/**
 * SysTick Timer Interrupt Handler. Counts milliseconds since start
 */
void SysTick_Handler(void) {
	msTicks++;
}

/**
 * CCAN Interrupt Handler. Calls the isr() API located in the CCAN ROM
 */
void CAN_IRQHandler(void) {
	LPC_CCAN_API->isr();
}

// -------------------------------------------------------------
// Public Functions and Members

const uint32_t OscRateIn = 0;

int8_t Board_SysTick_Init(void) {
	msTicks = 0;

	// Update the value of SystemCoreClock to the clock speed in hz
	SystemCoreClockUpdate();

	// Initialize SysTick Timer to fire interrupt at 1kHz
	return (SysTick_Config (SystemCoreClock / 1000));
}

void Board_LEDs_Init(void) {
	Chip_GPIO_Init(LPC_GPIO);
	Chip_GPIO_WriteDirBit(LPC_GPIO, LED0, true);
	Chip_GPIO_WriteDirBit(LPC_GPIO, LED1, true);
	Chip_GPIO_WriteDirBit(LPC_GPIO, 0, 6, true);
	Chip_GPIO_WriteDirBit(LPC_GPIO, 0,7,true);
	Chip_GPIO_SetPinState(LPC_GPIO, LED0, true);
	Chip_GPIO_SetPinState(LPC_GPIO, LED1, true);
	Chip_GPIO_SetPinState(LPC_GPIO, 0, 6, true);
	Chip_GPIO_SetPinState(LPC_GPIO, 0, 7, true);
}

void Board_LEDs_Off(void){
	Chip_GPIO_SetPinState(LPC_GPIO, LED0, false);
}

void Board_LEDs_On(void){
	Chip_GPIO_SetPinState(LPC_GPIO, LED0, true);
}

void Board_LEDtwo_Off(void){
	Chip_GPIO_SetPinState(LPC_GPIO, LED1, false);
}

void Board_LEDtwo_On(void){
	Chip_GPIO_SetPinState(LPC_GPIO, LED1, true);
}

void Board_UART_Init(uint32_t baudrate) {
	Chip_IOCON_PinMuxSet(LPC_IOCON, UART_RX_IOCON, (IOCON_FUNC1 | IOCON_MODE_INACT));	// Rx pin
	Chip_IOCON_PinMuxSet(LPC_IOCON, UART_TX_IOCON, (IOCON_FUNC1 | IOCON_MODE_INACT));	// Tx Pin

	Chip_UART_Init(LPC_USART);
	Chip_UART_SetBaud(LPC_USART, baudrate);
	Chip_UART_ConfigData(LPC_USART, (UART_LCR_WLEN8 | UART_LCR_SBS_1BIT | UART_LCR_PARITY_DIS));
	Chip_UART_SetupFIFOS(LPC_USART, (UART_FCR_FIFO_EN | UART_FCR_TRG_LEV2));
	Chip_UART_TXEnable(LPC_USART);
}

void Board_UART_Print(const char *str) {
	Chip_UART_SendBlocking(LPC_USART, str, strlen(str));
}

void Board_UART_Println(const char *str) {
	Board_UART_Print(str);
	Board_UART_Print("\r\n");
}

void Board_UART_PrintNum(const int num, uint8_t base, bool crlf) {
	static char str[32];
	itoa(num, str, base);
	Board_UART_Print(str);
	if (crlf) Board_UART_Print("\r\n");
}

void Board_UART_SendBlocking(const void *data, uint8_t num_bytes) {
	Chip_UART_SendBlocking(LPC_USART, data, num_bytes);
}

int8_t Board_UART_Read(void *data, uint8_t num_bytes) {
	return Chip_UART_Read(LPC_USART, data, num_bytes);
}

void CAN_baudrate_calculate(uint32_t baud_rate, uint32_t *can_api_timing_cfg)
{
	uint32_t pClk, div, quanta, segs, seg1, seg2, clk_per_bit, can_sjw;
	Chip_Clock_EnablePeriphClock(SYSCTL_CLOCK_CAN);
	pClk = Chip_Clock_GetMainClockRate();

	clk_per_bit = pClk / baud_rate;

	for (div = 0; div <= 15; div++) {
		for (quanta = 1; quanta <= 32; quanta++) {
			for (segs = 3; segs <= 17; segs++) {
				if (clk_per_bit == (segs * quanta * (div + 1))) {
					segs -= 3;
					seg1 = segs / 2;
					seg2 = segs - seg1;
					can_sjw = seg1 > 3 ? 3 : seg1;
					can_api_timing_cfg[0] = div;
					can_api_timing_cfg[1] =
						((quanta - 1) & 0x3F) | (can_sjw & 0x03) << 6 | (seg1 & 0x0F) << 8 | (seg2 & 0x07) << 12;
					return;
				}
			}
		}
	}
}

void Board_CAN_Init(uint32_t baudrate, void (*rx_callback)(uint8_t), void (*tx_callback)(uint8_t), void (*error_callback)(uint32_t), uint32_t (*CANOPEN_sdo_read)(uint16_t index, uint8_t subindex), uint32_t (*CANOPEN_sdo_write)(uint16_t index, uint8_t subindex, uint8_t *dat_ptr),uint8_t (*CANOPEN_sdo_req)(uint8_t length, uint8_t *req_ptr, uint8_t *length_resp, uint8_t *resp_ptr),CCAN_CANOPENCFG_T *canopen_cfg) {

	uint32_t can_api_timing_cfg[2];
	
	CCAN_CALLBACKS_T callbacks = {
		rx_callback,
		tx_callback,
		error_callback,
		CANOPEN_sdo_read,
		CANOPEN_sdo_write,
		NULL,
		NULL,
		CANOPEN_sdo_req,
	};

	CAN_baudrate_calculate(baudrate, can_api_timing_cfg);

	/* Initialize the CAN controller */
	LPC_CCAN_API->init_can(&can_api_timing_cfg[0], TRUE);
	/* Configure the CAN callback functions */
	LPC_CCAN_API->config_calb(&callbacks);

	LPC_CCAN_API->config_canopen(canopen_cfg);
	
	/* Enable the CAN Interrupt */
	NVIC_EnableIRQ(CAN_IRQn);
}

void Board_CANopen_Write(uint8_t node, uint16_t index, uint8_t subindex, uint8_t* data, uint8_t size){
	CCAN_MSG_OBJ_T msg_SDO;
	msg_SDO.mode_id = 0x600 + node;
	msg_SDO.mask = 0x00;
	msg_SDO.dlc = 8;
	msg_SDO.msgobj = 8;
	msg_SDO.data[0] = 1<<5 | (4-size)<<2 | 0x02 | 0x01;
	msg_SDO.data[1] = index & 0xFF;
	msg_SDO.data[2] = (index >> 8) & 0xFF;
	msg_SDO.data[3] = subindex;
	msg_SDO.data[4] = 0x00;
	msg_SDO.data[5] = 0x00;
	msg_SDO.data[6] = 0x00;
	msg_SDO.data[7] = 0x00;

	int i=0;
	for(i=4;i<size+4;i++){
		msg_SDO.data[i]=data[i-4];
	}
	
	LPC_CCAN_API->can_transmit(&msg_SDO);
	
}
