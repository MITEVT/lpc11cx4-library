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
