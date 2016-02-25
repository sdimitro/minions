#ifndef __MASTER_PROC_H
#define __MASTER_PROC_H

#include "mtc_queue.h"

struct mproc_state {
	struct queue *incoming;
	struct queue *results;
	pthread_t *worker_threads;
	int *kill_master;
	int workers;
};

struct task_desc *execute_task(struct task_desc *task);
void *worker_handler(void *data);

#endif /*__MASTER_PROC_H */
