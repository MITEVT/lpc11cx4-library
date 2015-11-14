#ifndef __BOARD_H_
#define __BOARD_H_

#include "chip.h"
#include "util.h"
#include <string.h>

// -------------------------------------------------------------
// Global Variables

extern const uint32_t OscRateIn; 
volatile uint32_t msTicks; 						// Running count of milliseconds since start

// -------------------------------------------------------------
// Configuration Macros


// -------------------------------------------------------------
// Pin Descriptions

#define LED0_PORT 2
#define LED0_PIN 10

#define LED1_PORT 0
#define LED1_PIN 6

#define UART_RX_PORT 1
#define UART_RX_PIN 6
#define UART_RX_IOCON IOCON_PIO1_6

#define UART_TX_PORT 1
#define UART_TX_PIN 7
#define UART_TX_IOCON IOCON_PIO1_7



// -------------------------------------------------------------
// Computed Macros

#define LED0 LED0_PORT, LED0_PIN
#define LED1 LED1_PORT, LED1_PIN

#define UART_RX UART_RX_PORT, UART_RX_PIN
#define UART_TX UART_TX_PORT, UART_TX_PIN

#define Board_LED_On(led) Chip_GPIO_SetPinState(LPC_GPIO, led, true)
#define Board_LED_Off(led) Chip_GPIO_SetPinState(LPC_GPIO, led, false)
 
// -------------------------------------------------------------
// Board Level Function Prototypes

int8_t Board_SysTick_Init(void);

void Board_LEDs_Init(void);

void Board_UART_Init(uint32_t baudrate);

/**
 * Transmit the given string through the UART peripheral (blocking)
 * 
 * @param str pointer to string to transmit
 * @note	This function will send or place all bytes into the transmit
 *			FIFO. This function will block until the last bytes are in the FIFO.
 */
void Board_UART_Print(const char *str);

/**
 * Transmit a string through the UART peripheral and append a newline and a linefeed character (blocking)
 * 
 * @param str pointer to string to transmit
 * @note	This function will send or place all bytes into the transmit
 *			FIFO. This function will block until the last bytes are in the FIFO.
 */
void Board_UART_Println(const char *str);

/**
 * Transmit a string containing a number through the UART peripheral (blocking)
 * 
 * @param num number to print
 * @param base number base
 * @param crlf append carraige return and line feed
 */
void Board_UART_PrintNum(const int num, uint8_t base, bool crlf);

/**
 * Transmit a byte array through the UART peripheral (blocking)
 * 
 * @param	data		: Pointer to data to transmit
 * @param	num_bytes	: Number of bytes to transmit
 * @note	This function will send or place all bytes into the transmit
 *			FIFO. This function will block until the last bytes are in the FIFO.
 */
void Board_UART_SendBlocking(const void *data, uint8_t num_bytes);

/**
 * Read data through the UART peripheral (non-blocking)
 * 
 * @param	data		: Pointer to bytes array to fill
 * @param	num_bytes	: Size of the passed data array
 * @return	The actual number of bytes read
 * @note	This function reads data from the receive FIFO until either
 *			all the data has been read or the passed buffer is completely full.
 *			This function will not block. This function ignores errors.
 */
int8_t Board_UART_Read(void *data, uint8_t num_bytes);

void Board_CAN_Init(uint32_t baudrate, void (*rx_callback)(uint8_t), void (*tx_callback)(uint8_t), void (*error_callback)(uint32_t));

void Board_DAC_Init(void);

void Board_DAC_SetVoltage(uint16_t millivolts);
#endif
