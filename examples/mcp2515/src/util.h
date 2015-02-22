#include "lpc_types.h"

void __reverse(char* begin,char* end);

char* itoa(int value, char* buffer, int base);

void uint8ToASCIIHex(uint8_t val, char* str);
void uint16ToASCIIHex(uint16_t val, char* str);
void uint32ToASCIIHex(uint32_t val, char* str);

uint16_t getIDFromBytes(uint8_t high, uint8_t low);