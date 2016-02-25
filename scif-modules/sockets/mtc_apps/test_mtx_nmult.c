#include "mtc_apps.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define WIDTH 3

/*
 * NOTICE:
 * I would have loved to develop a quick test case
 * that could actually verify that the multiplication
 * is actually done right. Unfortunately, this will
 * have to stay in the backlog for some time. For
 * the time being I can just print the result and
 * verify by hand using small matrices.
 */

static void init_mtx(float *matrix, int matrix_size)
{
	int i;
	for (i = 0; i < matrix_size; i++)
		matrix[i] = i;
}

static void print_mtx(float *matrix, int matrix_width)
{
	int i, matrix_size = matrix_width * matrix_width;
	for (i = 0; i < matrix_size; i++) {
		printf("%.1f ", matrix[i]);
		if ((i + 1) % matrix_width == 0) printf("\n");
	}
}

int main()
{
	int matrix_width = WIDTH,
	    matrix_size = WIDTH * WIDTH;
	void *params = malloc(sizeof(int) +
		3 * matrix_size * sizeof(float));

	*((int *) params) = matrix_width;
	float *A = (float *) (((char *) params) + sizeof(int));
	float *B = A + matrix_size;
	float *C = B + matrix_size;

	init_mtx(A, matrix_size);
	init_mtx(B, matrix_size);

	printf("Both Matrices A and B look like this:\n");
	print_mtx(A, matrix_width);
	printf("\n");

	naive_matrix_multiplication(params);

	printf("Result looks like this:\n");
	print_mtx(C, matrix_width);

	free(params);
	return 0;
}
