#ifndef __MTC_QUEUE_H
#define __MTC_QUEUE_H

#include <pthread.h>
#include <semaphore.h>

struct task_desc {
	int task_id;
	int task_type;
	int num_threads;
	void *params;
};

struct queue {
	struct task_desc **tasks; /* the actual queue */
	int capacity;             /* maximum number of task slots */
	int rear;                 /* tasks[rear % capacity] is the last item */
	int front;                /* tasks[(front+1) % capacity] is the first */
	pthread_mutex_t lock;     /* protects access to the queue */
	sem_t task_sem;           /* counts available tasks in queue */
	sem_t spaces_sem;         /* counts available task slots */
};

struct queue *create_queue(int size);
void dispose_queue(struct queue *q);
void enqueue(struct task_desc *task, struct queue *q);
struct task_desc *dequeue(struct queue *q);


#endif /* __MTC_QUEUE_H */

/*
 * NOTES:
 * [Code Optimization & Readability] Change **tasks to zero-length array
 * [Reasoning] Look at the Finite Producer-Consumer Solution of LBoS
 * [Issue] Make rear and front unsigned long maybe? possibly capacity too
 *         so nothing funky happens when converting during mod. If not
 *         then I need to explicity assign front & rear so they are bounded
 *         by the capacity of the queue as they used to be. (Look previous
 *         commits )
 */
