#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "scif_client_helper.h"

static void print_mtx(float *matrix, int matrix_width)
{
	printf("= Result Matrix:\n= ");
  int i, matrix_size = matrix_width * matrix_width;
  for (i = 0; i < matrix_size; i++) {
    printf("%.1f ", matrix[i]);
    if ((i + 1) % matrix_width == 0) printf("\n= ");
  }
}

int main(int argc, char *argv[])
{
	scif_epd_t epd;
	int bytes_sent, bytes_received, count;

	/* message state related variables */
	int *id, *type, *threads;
	size_t message_size, request, request_size;
	void *message, *params;

	/* do the standard open, bind, connect in SCIF */
	epd = scif_obc();

	/* Create request */
	request_size = sizeof(size_t);
	request = 1;

	printf("= About to send %zd bytes\n", request_size);

	/* Send message */
	bytes_sent = scif_send(epd, &request, request_size, 1);
	printf("= Sent %d bytes\n= Waiting for reply ...\n", bytes_sent);

	/* Receive size of the reply */
	bytes_received = scif_recv(epd, &message_size, sizeof(size_t), 1);
	printf("= Received %d bytes. Expecting a message of size %zu bytes\n",
		bytes_received, message_size);

	/* Receive the actual reply */
	message = malloc(message_size);
	count = scif_recv(epd, message, message_size, 1);
	bytes_received += count;
	printf("= Received %d bytes. Total bytes received: %d bytes\n",
		count, bytes_received);

	/*
	 * Extract the message received:
	 * --------------------------------------------------
	 * | type (int) | threads (int) | id (int) | params |
	 * --------------------------------------------------
	 */
	type = (int *) message;
	threads = type + 1;
	id = threads + 1;
	params = (void *) ((char *) id + sizeof(int));

	printf("= Content size: %zu bytes - Type: %d - Threads: %d - ID: %d\n",
		message_size, *type, *threads, *id);


	/* output results */
	int matrix_width, matrix_size;
	float *result;
	switch(*type) {
	case 1:
		printf("= Sleep duration left: %u\n", *((unsigned int *) params));
		break;
	case 2:
		matrix_width = *((int *) params);
		result = (float *) (((char *) params) + sizeof(int));
		print_mtx(result, matrix_width);
		break;
	case 3: case 4: case 6:
		matrix_width = *((int *) params);
		matrix_size = matrix_width * matrix_width;
		result = (float *) (((char *) params) + sizeof(int)
			+ 2 * matrix_size * sizeof(float));
		print_mtx(result, matrix_width);
		break;
	case 5:
		matrix_width = *((int *) params + 1);
		matrix_size = matrix_width * matrix_width;
		result = (float *) (((char *) params) + 2 * sizeof(int)
			+ 2 * matrix_size * sizeof(float));
		print_mtx(result, matrix_width);
		break;
	default:
		printf("= Dat shit cray!\n");
		break;
	}

	free(message);

	if (scif_close(epd) != 0) {
		fprintf(stderr, "scif_close failed with error %d\n", errno);
		exit(EXIT_FAILURE);
	}
	printf("= scif_close success\n");

	return EXIT_SUCCESS;
}
