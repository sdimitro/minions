#define _GNU_SOURCE
#include <sched.h>

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define CURRENT_THREAD 0

int
main(void)
{
	cpu_set_t set;
	CPU_ZERO(&set);
	CPU_SET(1, &set);
	int err = sched_setaffinity(CURRENT_THREAD, sizeof(set), &set);
	if (err == -1) {
		fprintf(stderr, "error: sched_setaffinity() returned %d\n", errno);
		exit(EXIT_FAILURE);
	}

	// int scheduler_priority = sched_get_priority_min(SCHED_FIFO);
	int scheduler_priority = sched_get_priority_max(SCHED_FIFO);
	if (scheduler_priority == -1) {
		fprintf(stderr, "error: sched_get_priority_max() returned %d\n", errno);
		exit(EXIT_FAILURE);
	}

	struct sched_param scheduler_parameters = {
		.sched_priority = scheduler_priority,
	};
	err = sched_setscheduler(CURRENT_THREAD, SCHED_FIFO, &scheduler_parameters);
	if (err != 0) {
		fprintf(stderr, "error: sched_setscheduler() returned %d\n", errno);
		exit(EXIT_FAILURE);
	}

	uint64_t j = 0;
	uint64_t max = ((uint64_t)UINT32_MAX) << 6;
	// printf("%ld - prio %d\n", max, scheduler_priority);
	for (uint64_t i = 0; i < max; i++) {
		j++;
	}
	exit(EXIT_SUCCESS);
}
