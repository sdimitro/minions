#ifndef __MTC_APPS_H
#define __MTC_APPS_H

void common_sleep(void *params);

/* sequential apps */
void matrix_zero(void *params);
void naive_matrix_multiplication(void *params);
void optimized_matrix_multiplication(void *params);
void blocking_matrix_multiplication(void *params);

/* multi-threaded apps */
void parallel_matrix_multiplication(int num_threads, void *params);

#endif /* __MTC_APPS_H */
