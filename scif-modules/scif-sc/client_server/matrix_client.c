#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define PROGNAME "matrix_client"

static void print_usage()
{
	fprintf(stderr, "Usage: %s [ip address] [port] [id] [type] [..<args>]\n",
		PROGNAME);
	exit(EXIT_FAILURE);
}

static void eval_args(int c, char *v[],
	int *type, int *matrix_width, int *secondary)
{
	if (c < 6) {
		print_usage();
	}

	*type = atoi(v[4]);
	switch(*type) {
	case 2: case 3: case 4:
		if (c != 6) print_usage();
		break;
	case 5: case 6:
		if (c != 7) print_usage();
		*secondary = atoi(v[6]);
		break;
	default:
		fprintf(stderr, "Available matrix operations are types: 2, 3, 4, 5, 6\n");
		print_usage();
	}

	*matrix_width = atoi(v[5]);
}

static void init_mtx(float *matrix, int matrix_size)
{
  int i;
  for (i = 0; i < matrix_size; i++)
    matrix[i] = i;
}

int main(int argc, char*argv[]) {

	/* connection related variables */
	int sockfd, port, bytes_sent;
	struct sockaddr_in servaddr;

	/* message state related variables */
	int input_type, matrix_width, secondary, matrix_size;
	int *id, *type, *threads, *bsize, *width;
	float *A, *B; /* *C is also allocated to store the result */ 
	size_t message_size;
	void *message;

	eval_args(argc, argv, &input_type, &matrix_width, &secondary);
	matrix_size = matrix_width * matrix_width;
	port = atoi(argv[2]);

	/* create socket and connect */
	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr(argv[1]);
	servaddr.sin_port = htons(port);

	connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));


	/*
	 * calculate message_size:
	 * The first 4 fields of the message
	 * The matrix width - first or second field of params
	 * The block size   - first field of params (applicable only if type 5)
	 * The size of the matrices (or matrix if type 2)
	 */
	message_size = sizeof(size_t) + 3 * sizeof(int);
	message_size += sizeof(int);
	if (input_type == 5) { message_size += sizeof(int); }
	if (input_type == 2)
		message_size += matrix_size * sizeof(float);
	else
		message_size += 3 * matrix_size * sizeof(float);

	/*
	 * setup message to send:
   * --------------------------------------------------------------------------
	 * | message_size (size_t) | type (int) | threads (int) | id (int) | params |
   * --------------------------------------------------------------------------
	 */
	message = malloc(message_size);

	*((size_t *) message) = message_size;
	type = (int *) ((char *) message + sizeof(size_t));
	*type = input_type;
	threads = type + 1;
	*threads = (*type == 6) ? secondary : 0;
	id = threads + 1;
	*id = atoi(argv[3]);

	if (*type == 5) { bsize = id + 1; *bsize = secondary; width = bsize + 1; }
	else { width = id + 1; }
	*width = matrix_width;

	A = (float *) (((char *) width) + sizeof(int));
	init_mtx(A, matrix_size);
	if (*type != 2) {
		B = A + matrix_size;
		init_mtx(B, matrix_size);
	}

	printf("= Content size: %zu bytes - Type: %d - Threads: %d - ID: %d\n",
		message_size, *type, *threads, *id);
	printf("= About to send %zd bytes\n", message_size);

	/* Send message */
	bytes_sent = sendto(sockfd, message, message_size, 0,
	       (struct sockaddr *)&servaddr, sizeof(servaddr));
	printf("= Sent %d bytes\n", bytes_sent);

	free(message);
	close(sockfd);

	return 0;
}
