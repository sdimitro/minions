#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "scif_client_helper.h"

int main(int argc, char *argv[])
{
	scif_epd_t epd;
	int bytes_sent;
	size_t message_size, message;

	/* do the standard open, bind, connect in SCIF */
	epd = scif_obc();

	/* The contents of the message is basically 0 */
	message_size = sizeof(size_t);
	message = 0;

	printf("= About to send %zu bytes\n", message_size);

	/* Send message */
	bytes_sent = scif_send(epd, &message, message_size, 0); /* May need to set flags to 1 */
	printf("= Sent %d bytes\n", bytes_sent);

	if (scif_close(epd) != 0) {
		fprintf(stderr, "scif_close failed with error %d\n", errno);
		exit(EXIT_FAILURE);
	}
	printf("= scif_close success\n");

	return EXIT_SUCCESS;
}
