#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define SLEEP_JOB 1

int main(int argc, char*argv[]) {

	/* connection related variables */
	int sockfd, port, bytes_sent;
	struct sockaddr_in servaddr;

	/* message state related variables */
	unsigned int *sleep_time;
	int *id, *type, *threads;
	size_t message_size;
	void *message;

	if (argc != 5) {
		fprintf(stderr, "Usage: %s [ip address] [port] [id] [sleep time]\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	port = atoi(argv[2]);

	/* create socket and connect */
	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr(argv[1]);
	servaddr.sin_port = htons(port);

	connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));


	/*
	 * setup message to send:
   * --------------------------------------------------------------------------
	 * | message_size (size_t) | type (int) | threads (int) | id (int) | params |
   * --------------------------------------------------------------------------
	 */
	message_size = sizeof(int) * 3 + sizeof(unsigned int);
	message = malloc(sizeof(size_t) + message_size);

	*((size_t *) message) = message_size;
	type = (int *) ((char *) message + sizeof(size_t));
	*type = SLEEP_JOB;
	threads = type + 1; *threads = 0;
	id = threads + 1; *id = atoi(argv[3]);
	sleep_time = (unsigned int *) ((char *) id + sizeof(int));
	*sleep_time = atoi(argv[4]);

	printf("= Content size: %zu bytes - Type: %d - Threads: %d - ID: %d - Sleep Duration: %u\n",
		message_size, *type, *threads, *id, *sleep_time);
	printf("= About to send %zd bytes\n", sizeof(size_t) + message_size);

	/* Send message */
	bytes_sent = sendto(sockfd, message, sizeof(size_t) + message_size,0,
	       (struct sockaddr *)&servaddr, sizeof(servaddr));
	printf("= Sent %d bytes\n", bytes_sent);

	free(message);
	close(sockfd);

	return 0;
}
