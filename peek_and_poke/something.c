#include <stdio.h>
#include <stdint.h>
int main() {
	printf("RTC: %zu\n", *((uint32_t *)0x40024000));
	return 0;
}
