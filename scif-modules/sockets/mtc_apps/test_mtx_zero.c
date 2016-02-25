#include "mtc_apps.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define WIDTH 10

int main()
{
	int matrix_width = WIDTH,
	    matrix_size = WIDTH * WIDTH;
	void *params = malloc(sizeof(int) +
		matrix_size * sizeof(float));

	*((int *) params) = matrix_width;
	float *matrix = (float *) (((char *) params) + sizeof(int));

	/* because I'm feeling stupid today */
	matrix[matrix_width] = 1.0;

	matrix_zero(params);

	assert(matrix[matrix_width] != 1.0);

	int i, flag = 0;
	for (i = 0; i < matrix_size; i++)
		if (matrix[i] != 0.0) { flag++; break; }

	free(params);
	return flag;
}
