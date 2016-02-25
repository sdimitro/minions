#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char*argv[]) {

	int sockfd, port, bytes_sent;
	struct sockaddr_in servaddr;
	size_t message_size, message;

	if (argc != 3) {
		fprintf(stderr, "Usage: %s [ip address] [port]\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	port = atoi(argv[2]);

	/* create socket and connect */
	sockfd = socket(AF_INET, SOCK_STREAM,0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr(argv[1]);
	servaddr.sin_port = htons(port);

	connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));


	/* The contents of the message is basically 0 */
	message_size = sizeof(size_t);
	message = 0;

	printf("= About to send %zu bytes\n", message_size);

	/* Send message */
	bytes_sent = sendto(sockfd, &message, message_size, 0,
	       (struct sockaddr *)&servaddr, sizeof(servaddr));
	printf("= Sent %d bytes\n", bytes_sent);

	close(sockfd);

	return 0;
}
