#include "mtc_apps.h"

#include <stdio.h>
#include <assert.h>

#define SLEEP_DURATION 10

int main()
{
	int sleep_left = SLEEP_DURATION;

	printf("sleep duration: %d\n", sleep_left);
	printf("entering common_sleep() ...\n");

	common_sleep((void *) &sleep_left);

	printf("returned.\n");
	printf("sleep Duration left: %d\n", sleep_left);

	/* Pointless (only for the curious) */
	assert(sleep_left == 0);

	return sleep_left;
}
