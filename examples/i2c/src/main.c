/**************************
* 	I2C Master Example    *
***************************/


#include "chip.h"
#include "util.h"
#include <string.h>

const uint32_t OscRateIn = 0000000;

#define LED0 2, 5
#define LED1 0, 6
#define LED2 0, 7
#define LED3 2, 9

#define SCL 0, 4
#define SDA 0, 5

#define SLAVEADDR 0x64

#define print(str) {Chip_UART_SendBlocking(LPC_USART, str, strlen(str));}
#define println(str) {print(str); print("\r\n");}
#define printnum(val, buf, base) {itoa(val, buf, base); print(buf);}

volatile uint32_t msTicks;

uint8_t rx_buf[8];
char print_buf[32];
char cmd_buf[20];
uint8_t cmd_count = 0;

#define I2C_BUF_SIZE 10

I2C_XFER_T i2c_xfer_obj;
uint8_t i2c_tx_buf[I2C_BUF_SIZE];
uint8_t i2c_rx_buf[I2C_BUF_SIZE]; 

void SysTick_Handler(void) {
	msTicks++;
}

void I2C_IRQHandler(void) {
	print("\r\n*I2C Interrupt STAT: 0x");
	printnum(LPC_I2C->STAT, print_buf, 16);
	println("*")

	if (LPC_I2C->STAT == I2C_I2STAT_M_TX_START) { // Entered Master Mode. Load Slave Addr with Write
		println("Master Transmit Mode");
		LPC_I2C->DAT = i2c_xfer_obj.slaveAddr << 1 | 0x00; // Begin write mode
		LPC_I2C->CONCLR = I2C_I2CONCLR_SIC;
	} else if (LPC_I2C->STAT == I2C_I2STAT_M_TX_SLAW_ACK) {
		print("SLA+W trans, ACK. ");
		if (i2c_xfer_obj.txSz > 0) {
			LPC_I2C->DAT = i2c_xfer_obj.txBuff[0];
			i2c_xfer_obj.txBuff++;
			i2c_xfer_obj.txSz--;
			LPC_I2C->CONCLR = I2C_I2CONCLR_SIC;
			println("trans");
		} else {
			LPC_I2C->CONSET = I2C_I2CONSET_STO;
			println("stop");
		}
	} else if (LPC_I2C->STAT == I2C_I2STAT_M_TX_SLAW_NACK) {
		println("SLA+W trans, NACK");
	} else if (LPC_I2C->STAT == I2C_I2STAT_M_TX_DAT_ACK) {
		println("DAT trans, ACK");
	} else if (LPC_I2C->STAT == I2C_I2STAT_M_TX_DAT_NACK) {
		println("DAT trans, NACK")
	} else {
		println("Fuck");
	}
}

int main(void)
{

	SystemCoreClockUpdate();

	if (SysTick_Config (SystemCoreClock / 1000)) {
		//Error
		while(1);
	}

	// Initialize UART

	Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO1_6, (IOCON_FUNC1 | IOCON_MODE_INACT));/* RXD */
	Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO1_7, (IOCON_FUNC1 | IOCON_MODE_INACT));/* TXD */

	Chip_UART_Init(LPC_USART);
	Chip_UART_SetBaud(LPC_USART, 57600);
	Chip_UART_ConfigData(LPC_USART, (UART_LCR_WLEN8 | UART_LCR_SBS_1BIT | UART_LCR_PARITY_DIS));
	Chip_UART_SetupFIFOS(LPC_USART, (UART_FCR_FIFO_EN | UART_FCR_TRG_LEV2));
	Chip_UART_TXEnable(LPC_USART);

	print(">");
	cmd_count = 0;
	cmd_buf[0] = '\0';

	// Turn on Power LED

	Chip_GPIO_Init(LPC_GPIO);
	Chip_GPIO_WriteDirBit(LPC_GPIO, LED0, 1);
	Chip_GPIO_SetPinState(LPC_GPIO, LED0, 1);

	// Initialize I2C

	Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO0_4, IOCON_FUNC1); // SCL
	Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO0_5, IOCON_FUNC1); // SDA

	LPC_SYSCTL->SYSAHBCLKCTRL |= 0x20; 	// Enable clock and power to I2C block
	LPC_SYSCTL->PRESETCTRL |= 0x02; 	// De-assert I2C block reset signal

	NVIC_ClearPendingIRQ(I2C0_IRQn);	// Clear any pending I2C interrupts
	NVIC_EnableIRQ(I2C0_IRQn);			// Enable I2C interrupts

	LPC_I2C->CONSET = 0x40; 			// Enable I2C and disable slave mode

	LPC_I2C->SCLH = 60;					// Set to 100kHz
	LPC_I2C->SCLL = 60;

	i2c_xfer_obj.txBuff = i2c_tx_buf;
	i2c_xfer_obj.rxBuff = i2c_rx_buf;

	// Main loop
    while(1) {
		uint8_t count;
		if ((count = Chip_UART_Read(LPC_USART, rx_buf, 8)) != 0) {
			Chip_UART_SendBlocking(LPC_USART, rx_buf, count);
			if (rx_buf[0] == '\r' || rx_buf[0] == '\n') {
				if (cmd_buf[0] == 'h') {
					println("s - I2CSTAT");
					println("a - Read Register at address 0");
					print("h - Help Prompt");

				} else if (cmd_buf[0] == 's') {
					print("I2CSTAT: 0x");
					printnum(LPC_I2C->STAT, print_buf, 16);
				} else if (cmd_buf[0] == 'a') {
					i2c_xfer_obj.slaveAddr = SLAVEADDR;
					i2c_tx_buf[0] = 0x00;
					i2c_xfer_obj.txBuff = i2c_tx_buf;
					i2c_xfer_obj.txSz = 1;
					i2c_xfer_obj.rxSz = 0;

					LPC_I2C->CONSET = 0x20;
				} else if (cmd_buf[0] == '\0') {

				} else {
					print("Unknown command.");
				}
				println("");
				print(">");
				cmd_buf[0] = '\0';
				cmd_count = 0;
			} else {
				memcpy(cmd_buf + cmd_count, rx_buf, count);
			}
			
        
		}
	}

	return 0;
}
