#include <errno.h>
#include <stdio.h>
#include <stdlib.h>


#include "scif_client_helper.h"

scif_epd_t scif_obc(void)
{
	scif_epd_t epd;
	struct scif_portID portID;
	int connection_port, request_port;
	int err, tries = MAX_TRIES;

	/* create "socket" and connect */
	request_port = LOCAL_PORT;
	portID.node = 1; /* 1 for MidWay, 0 for RunAway */
	portID.port = PEER_PORT;

	if ((epd = scif_open()) == SCIF_OPEN_FAILED) {
		fprintf(stderr, "= scif_open failed with error %d\n", (int) epd);
		exit(EXIT_FAILURE);
	}

	if ((connection_port = scif_bind(epd, request_port)) < 0) {
		fprintf(stderr, "= scif_bind failed with error %d\n",
			connection_port);
		exit(EXIT_FAILURE);
	}
	printf("= scif_bind to port %d success\n", connection_port);

__retry:
	if ((err = scif_connect(epd, &portID)) < 0) {
		if ((errno == ECONNREFUSED) && (tries > 0)) {
			printf("= connection to node %d failed : trial %d\n",
				portID.node, tries);
			tries--;
			sleep(1);
			goto __retry;
		}
			fprintf(stderr, "= scif_connect failed with error %d\n", errno);
			exit(EXIT_FAILURE);
	}
	printf("= conect to node %d success\n", portID.node);

	return epd;
}
