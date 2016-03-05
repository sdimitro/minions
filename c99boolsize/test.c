#include <stdbool.h>
#include <stdio.h>

int
main(void)
{
	printf("sizeof(bool)  = %u\n", sizeof(bool));
	printf("sizeof(_Bool) = %u\n", sizeof(_Bool));
	return 0;
}

