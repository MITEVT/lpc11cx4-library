#include "util.h"

void __reverse(char* begin,char* end) {
	char temp;

	while (end  >begin)
	{
		temp = *end;
		*end-- = *begin;
		*begin++ = temp;
	}
}

void uint8ToASCIIHex(uint8_t val, char* str) {
	uint8_t high = val >> 4;
	if (high < 10) {
		str[0] = high | 0x30;
	} else {
		high -= 9;
		str[0] = high | 0x40;
	}

	uint8_t low = val & 0x0F;
	if (low < 10) {
		str[1] = low | 0x30;
	} else {
		low -= 9;
		str[1] = low | 0x40;
	}
}

void uint16ToASCIIHex(uint16_t val, char* str) {
	uint8ToASCIIHex(val >> 8, str);
	uint8ToASCIIHex(val & 0x00FF, str + 2);
}

void uint32ToASCIIHex(uint32_t val, char* str) {
	uint16ToASCIIHex(val >> 16, str);
	uint16ToASCIIHex(val & 0xFFFF, str + 4);
}

uint16_t getIDFromBytes(uint8_t high, uint8_t low) {
	return (low >> 5) | (high << 3);
}

char* itoa(int value, char* buffer, int base) {
	static const char digits[] = "0123456789abcdef";

	char* buffer_copy = buffer;
	int32_t sign = 0;
	int32_t quot, rem;

	if ((base >= 2) && (base <= 16))				// is the base valid?
	{
		if (base == 10 && (sign = value) < 0)// negative value and base == 10? store the copy (sign)
			value = -value;					// make it positive
		do {
			quot = value / base;				// calculate quotient and remainder
			rem = value % base;
			*buffer++ = digits[rem];		// append the remainder to the string
		} while ((value = quot));				// loop while there is something to convert

		if (sign<0)							// was the value negative?
			*buffer++ = '-';					// append the sign

		__reverse(buffer_copy, buffer-1);		// reverse the string
	}

	*buffer='\0';
	return buffer_copy;
}