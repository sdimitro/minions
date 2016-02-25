#include "master_proc.h"
#include "mtc_apps/mtc_apps.h" /* need to fix that; see Makefile */

struct task_desc *execute_task(struct task_desc *task)
{
	switch (task->task_type) {
	case 0:
		/* STUB */
		break;
	case 1:
		common_sleep(task->params);
		break;
	case 2:
		matrix_zero(task->params);
		break;
	case 3:
		naive_matrix_multiplication(task->params);
		break;
	case 4:
		optimized_matrix_multiplication(task->params);
		break;
	case 5:
		blocking_matrix_multiplication(task->params);
		break;
	case 6:
		parallel_matrix_multiplication(task->num_threads, task->params);
		break;
	default:
		/* Error not supported! */
		break;
	}
	
	return task;
}

void *worker_handler(void *data)
{
  struct mproc_state *mps = (struct mproc_state *) data;
	struct task_desc *task;

	while(!(*(mps->kill_master))) {
		task = dequeue(mps->incoming);
		task = execute_task(task);
		enqueue(task, mps->results);
	}

	return NULL;
}


/*
 * NOTES:
 * TODO: switch case numbers above should be in an enum
 */
