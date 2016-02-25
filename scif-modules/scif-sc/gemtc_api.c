#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gemtc_api.h"
#include "mtc_queue.h"
#include "master_proc.h"

#include <assert.h>

struct mproc_state *mps;

void gemtc_setup(int queue_size, int workers)
{
	assert(queue_size > 0);
	assert(workers > 0);

	int *kill_master = (int *) malloc(sizeof(int));
	*kill_master = 0;

	mps = (struct mproc_state *) malloc(sizeof(struct mproc_state));
	mps->incoming = create_queue(queue_size);
	mps->results = create_queue(queue_size);
	mps->kill_master = kill_master;
	mps->workers = workers;
	mps->worker_threads = (pthread_t *) malloc(sizeof(pthread_t) * workers);

	int t;
	for (t = 0; t < workers; t++)
		pthread_create(&mps->worker_threads[t], NULL, worker_handler, (void *)mps);
}

void gemtc_cleanup()
{
	*(mps->kill_master) = 1;

	int t;
	for (t = 0; t < mps->workers; t++)
		pthread_cancel(mps->worker_threads[t]);

	for (t = 0; t < mps->workers; t++)
		pthread_join(mps->worker_threads[t], NULL);

	dispose_queue(mps->incoming);
	dispose_queue(mps->results);

	free(mps->worker_threads);
	free(mps->kill_master);
	free(mps);
}

void gemtc_push(int type, int threads, int id, void *params)
{
	struct task_desc *task = (struct task_desc *) malloc(sizeof(struct task_desc));	
	task->task_id = id;
	task->task_type = type;
	task->num_threads = threads;
	task->params = params;

	enqueue(task, mps->incoming);
}

void gemtc_poll(int *id, void **params)
{
	struct task_desc *task;

	task = dequeue(mps->results);

	assert(task != NULL);
	
	/* pass the pointers with the results */
	*id = task->task_id;
	*params = task->params;

	/* cleanup task */
	free(task);
}

void *gemtc_malloc(unsigned int nbytes)
{
	return malloc(nbytes);
}

void gemtc_free(void *loc) {
	free(loc);
}

/* For the memcpy functions use Xeon Phi's API */
void gemtc_memcpy_host2dev(void *host, void *device, size_t size)
{
	assert(host != 0);
	assert(device != 0);

	memcpy(host, device, size);
}

void gemtc_memcpy_dev2host(void *device, void *host, size_t size)
{
	assert(device != 0);
	assert(host != 0);

	memcpy(device, host, size);
}
