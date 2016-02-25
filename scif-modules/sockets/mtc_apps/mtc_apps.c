#include "mtc_apps.h"

#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>

void common_sleep(void *params)
{
	/*
	 * Parameters should come in the following form:
	 * [1] Sleep duration
	 */
	unsigned int *sleep_left = (unsigned int *) params;

	/*
	 * When the statement below executes params will
	 * be pointing to an unsigned integer value. This
   * value will either be 0 if the requested time
	 * elapsed, or the number of seconds left to sleep
	 * if the call was interrupted by a signal handler.
	 * See manual page of SLEEP(3).
	 */
	*sleep_left = sleep(*sleep_left);
}

void matrix_zero(void *params)
{
	/*
	 * Parameters should come in the following form:
	 * [1] width of matrix
	 * [2] matrix to be zeroed out
	 */
	int width = *((int *)params);
	float *M = (float *) (((char *)params) + sizeof(int));

	int matrix_size = width * width;

	int i;
	for (i = 0; i < matrix_size; i++)
		M[i] = 0.0;
}

void naive_matrix_multiplication(void *params)
{
	/*
	 * Parameters should come in the following form:
	 * [1] width of matrix
	 * [2] first matrix to be multiplied  (Matrix A)
	 * [3] second matrix to be multiplied (Matrix B)
	 * [4] result matrix (Matrix C)
	 */
	int width = *((int *)params);
	float *A = (float *) (((char *)params) + sizeof(int));
	float *B = A + width * width;
	float *C = B + width * width;

	float sum;
	int i, j, k;
	for (i = 0; i < width; i++)
		for (j = 0; j < width; j++) {
			sum = 0.0;
			for (k = 0; k < width; k++)
				sum += A[i * width + k] * B[k * width + j];
			C[i * width + j] = sum;
		}
}

void optimized_matrix_multiplication(void *params)
{
	/*
	 * Parameters should come in the following form:
	 * [1] width of matrix
	 * [2] first matrix to be multiplied  (Matrix A)
	 * [3] second matrix to be multiplied (Matrix B)
	 * [4] result matrix (Matrix C)
	 *
	 * Warning: This function assumes that Matrix C
	 *	entries are all zero initially.
	 */
	int width = *((int *)params);
	float *A = (float *) (((char *)params) + sizeof(int));
	float *B = A + width * width;
	float *C = B + width * width;

	float r;
	int i, k, j;
	for (i = 0; i < width; i++)
		for (k = 0; k < width; k++) {
			r = A[i * width + k];
			for (j = 0; j < width; j++)
				C[i * width + j] += r * B[k * width + j];
		}
}

void blocking_matrix_multiplication(void *params)
{
	/*
	 * Parameters should come in the following form:
	 * [1] block_size
	 * [2] width of matrix
	 * [3] first matrix to be multiplied  (Matrix A)
	 * [4] second matrix to be multiplied (Matrix B)
	 * [5] result matrix (Matrix C)
	 *
	 * Warning: This function assumes that Matrix C
	 *	entries are all zero initially. It also
	 *	assumes that the width is an integral
	 *	multiple of block_size.
	 */
	int block_size = *((int *)params);
	int width = *((int *) (((char *)params) + sizeof(int)));
	float *A = (float *) (((char *)params) + 2 * sizeof(int));
	float *B = A + width * width;
	float *C = B + width * width;

	int i, j, k, kk, jj;
	float sum;
	int en = block_size * (width / block_size);

	/* I am very sorry for what is about to come */

	for (kk = 0; kk < en; kk += block_size)
		for (jj = 0; jj < en; jj += block_size)
			for (i = 0; i < width; i++)
				for (j = jj; j < (jj + block_size); j++) {
					sum = C[i * width + j];
					for (k = kk; k < (kk + block_size); k++)
						sum += A[i * width + k] * B[k * width + j];
					C[i * width + j] = sum;
				}
}

/* ==== PARALLEL MATRIX MULTIPLICATION === */

struct pmm_worker_info {
	int id;
	int num_threads;
	void *params;
};

void * pmm_worker(void *arg)
{
	struct pmm_worker_info *info = (struct pmm_worker_info *) arg;
	void *params = info->params;

	int width = *((int *)params);
	float *A = (float *) (((char *)params) + sizeof(int));
	float *B = A + width * width;
	float *C = B + width * width;

	float sum;
	int i, j, k;
	for (i = info->id; i < width; i += info->num_threads)
		for (j = 0; j < width; j++) {
			sum = 0.0;
			for (k = 0; k < width; k++)
				sum += A[i * width + k] * B[k * width + j];
			C[i * width + j] = sum;
		}


	return NULL;
}

void parallel_matrix_multiplication(int num_threads, void *params)
{
	/*
	 * Parameters should come in the following form:
	 * [1] width of matrix
	 * [2] first matrix to be multiplied  (Matrix A)
	 * [3] second matrix to be multiplied (Matrix B)
	 * [4] result matrix (Matrix C)
	 */


	/*
	 * Take care of the case where we have more
	 * threads that we actually need.
	 */
	int width = *((int *)params);
	if (num_threads > width)
		num_threads = width;


	pthread_t *workers;
	struct pmm_worker_info *thread_info;

	workers = (pthread_t *) malloc(sizeof(pthread_t) * num_threads);
	thread_info = (struct pmm_worker_info *)
		malloc(sizeof(struct pmm_worker_info) * num_threads);

	int i;
	for (i = 0; i < num_threads; i++) {
		thread_info[i].id = i;
		thread_info[i].num_threads = num_threads;
		thread_info[i].params = params;
		pthread_create(&workers[i], NULL, pmm_worker, (void *) &thread_info[i]);
	}

	/* wait for threads */
	for (i = 0; i < num_threads; i++)
		pthread_join(workers[i], NULL);

	/* cleanup */
	free(workers);
	free(thread_info); /* Will definitely need to valgrind this */
}

/*
 * NOTES:
 * [Reasoning] Blocking matrix multiplication info:
 * 	http://csapp.cs.cmu.edu/2e/waside/waside-blocking.pdf
 * 	https://software.intel.com/en-us/articles/how-to-
 * 	use-loop-blocking-to-optimize-memory-use-on-32-bit-intel-architecture
 * [Capacity] May have to consider changing some ints
 * 	to (unsigned?) longs
 * [Optimization] On matrix mult (optimized version) could
 * 	also try kij besides ikj
 * [Hardware?] May need to change floats to doubles?
 */
