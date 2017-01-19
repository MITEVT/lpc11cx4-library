#ifndef _LC1024_H_
#define _LC1024_H_

#define RD_STATUS_REG_INSTR 0x5
#define WRITE_DISABLE_INSTR 0x4
#define READ_MEM_INSTR 0x3
#define WRITE_MEM_INSTR 0x2
#define WRITE_ENABLE_INSTR 0x6

// USES LPC_SSP1
void LC1024_Init(LPC_SSP_T *pSSP, uint32_t baud, uint8_t cs_gpio, uint8_t cs_pin);

// address can be array of atmost at most size 3 (uses only first three bytes)
void LC1024_ReadStatusReg(uint8_t* data);
void LC1024_WriteEnable(void);
void LC1024_WriteDisable(void);
void LC1024_ReadMem(uint8_t *address, uint8_t *rcv_buf, uint8_t len);
void LC1024_WriteMem(uint8_t *address, uint8_t *data, uint8_t len);
#endif
