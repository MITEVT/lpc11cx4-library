#ifndef _UTIL_H_
#define _UTIL_H_

#include "lpc_types.h"

void __reverse(char* begin,char* end);

char* itoa(int value, char* buffer, int base);

uint16_t getIDFromBytes(uint8_t high, uint8_t low);


#endif