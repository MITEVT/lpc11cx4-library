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
#define LED0_PIN 5

#define LED1_PORT 2
#define LED1_PIN 9

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

void Board_LEDs_On(void);

void Board_LEDs_Off(void);

void Board_LEDtwo_On(void);

void Board_LEDtwo_Off(void);

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

/**
 * Initialize CAN and CANopen
 *
 * @param	baudrate	: The baudrate for CAN
 * @param	canopen_cfg	: The CANopen config data
 *
 */
void Board_CAN_Init(uint32_t baudrate, CCAN_CANOPENCFG_T *canopen_cfg);

/**
 * Writes the designated data to a specific node's identified spot in the object dictionary
 *
 * @param	node		: The target server node
 * @param	index		: Tier 1 of the object dictionary ID
 * @param	subindex	: Tier 2 of the object dictionary ID
 * @param	data		: The new value(s) for the object dictionary
 * @param	size		: The number of values
 *
 */
void Board_CANopen_Write(uint8_t node, uint16_t index, uint8_t subindex, uint8_t *data, uint8_t size);

/**
 * Requests to read the designated node's part of the object dictionary
 *
 * @param	node		: The target server node
 * @param	index		: Tier 1 of the object dictionary ID
 * @param	subindex	: Tier 2 of the object dictionary ID
 * @param	data		: A pointer to where the data should be stored
 *
 */
void Board_CANopen_Read(uint8_t node, uint16_t index, uint8_t subindex, uint8_t *data);

/**
 * Displays CAN Errors, if they exist
 */
void Board_CAN_UART_DisplayError(void);

/**
 * Checks for if there is new CANopen data, then changes the flag back to no new data
 *
 * @return	True if there is new data, otherwise false
 *
 */
bool Board_CANopen_NewData(void);


#endif
