#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "scif_client_helper.h"

#define SLEEP_JOB 1

int main(int argc, char *argv[])
{
	scif_epd_t epd;
	int bytes_sent, count;

	/* message state related variables */
	unsigned int *sleep_time;
	int *id, *type, *threads;
	size_t message_size;
	void *message;

	if (argc != 3) {
		fprintf(stderr, "Usage: %s [id] [sleep time]\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	/* create socket and connect */
	epd = scif_obc();

	/*
	 * setup message to send:
	 * * --------------------------------------------------------------------------
	 * | message_size (size_t) | type (int) | threads (int) | id (int) | params |
	 * * --------------------------------------------------------------------------
	 */
	message_size = sizeof(int) * 3 + sizeof(unsigned int);
	message = malloc(sizeof(size_t) + message_size);

	*((size_t *) message) = message_size;
	type = (int *) ((char *) message + sizeof(size_t));
	*type = SLEEP_JOB;
	threads = type + 1; *threads = 0;
	id = threads + 1; *id = atoi(argv[1]);
	sleep_time = (unsigned int *) ((char *) id + sizeof(int));
	*sleep_time = atoi(argv[2]);

	printf("= Content size: %zu bytes - Type: %d - Threads: %d - ID: %d - Sleep Duration: %u\n",
		message_size, *type, *threads, *id, *sleep_time);
	printf("= About to send %zd bytes\n", sizeof(size_t) + message_size);

	/* Send message */
	bytes_sent = scif_send(epd, message, sizeof(size_t), 1);
	printf("= Sent %d bytes\n", bytes_sent);
	count = scif_send(epd, (((char *)message) +sizeof(size_t)), message_size, 1);
	bytes_sent += count;
	printf("= Sent %d bytes more. Total: %d bytes\n", count, bytes_sent);
	count = scif_recv(epd, &message_size, sizeof(size_t), 1);
	printf("= Received ACK: %s\n", (count == 0) ? "Failed" : "Success");

	free(message);

	if (scif_close(epd) != 0) {
		fprintf(stderr, "scif_close failed with error %d\n", errno);
		exit(EXIT_FAILURE);
	}
	printf("= scif_close success\n");

	return EXIT_SUCCESS;
}
