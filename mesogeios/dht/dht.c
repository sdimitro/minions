#define _POSIX_C_SOURCE 1

#include "ss.h"
#include "sbuf.h"
#include "ht.h"
#include "msg.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <pthread.h>

#define HTSZ 65536

#define UNUSED(x) (void)(x)


typedef struct {
	int port;
	char *host;
} peer_t;

typedef struct {
	peer_t *list;
	int size;
} peer_list_t;

sbuf_t sbuf;
ht_t *h;

static void respond_request(int cfd)
{
	char message[MSG_BYTES], command,
			 key[KEY_BYTES], value[VAL_BYTES];

	recv(cfd, message, MSG_BYTES, 0);
	decode_message(message, &command, key, value);

	if (command == 'd') {
		ht_set(h, key, "NOVAL");
		send(cfd, "OK", 3, 0);
	} else if (command == 'g') {
		char *v = (char *) calloc(VAL_BYTES, sizeof(char));
		char *ret = ht_get(h, key);

		if (ret == NULL || ret[0] == '\0')
			strncpy(v, "NOVAL", VAL_BYTES);
		else
			strncpy(v, ret, VAL_BYTES);

		send(cfd, v, strlen(v) + 1, 0);
		free(v);
	} else if (command == 'p') {
		ht_set(h, key, value);
		send(cfd, "OK", 3, 0);
	} else {
		printf("incorrect header: %c\n", command);
	}
}

void *thread(void *vargp)
{
	UNUSED(vargp);
	pthread_detach(pthread_self());
	while(1) {
		int *cfd_ptr = (int *) sbuf_remove(&sbuf);
		respond_request(*cfd_ptr);
		close(*cfd_ptr);
		free(cfd_ptr);
	}
}

static void dht_daemon(int nthreads, int qsize, int port)
{
	int listenfd, connfd, i;
	struct sockaddr_in clientaddr;
	socklen_t clientlen = sizeof(struct sockaddr_in);
	pthread_t tid;

	UNUSED(listenfd);
	UNUSED(clientaddr);
	UNUSED(clientlen);

	h = ht_create(HTSZ);
	sbuf_init(&sbuf, qsize);
	listenfd = open_listenfd(port);

	for (i = 0; i < nthreads; i++)
		pthread_create(&tid, NULL, thread, NULL);

	while (1) {
		connfd = accept(listenfd, (SA *) &clientaddr, &clientlen);
		int *cfd_ptr = (int *) malloc(sizeof(int));
		*cfd_ptr = connfd;
		sbuf_insert(&sbuf, cfd_ptr);
	}
}

int main(int c, char *v[])
{
	if (c != 4) {
		printf("usage: %s <peer port> "
		       "<queue size> <# of threads>\n", v[0]);
		return -1;
	}
	dht_daemon(atoi(v[3]), atoi(v[2]), atoi(v[1]));

	return 0;
}

