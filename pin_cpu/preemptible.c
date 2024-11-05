#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

int
main(void)
{
	uint64_t j = 0;
	uint64_t max = ((uint64_t)UINT32_MAX) << 6;
	for (uint64_t i = 0; i < max; i++) {
		j++;
	}
	j = 0;
	return j;
}
