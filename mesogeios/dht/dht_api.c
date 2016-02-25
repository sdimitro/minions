#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>

#include "dht_conf.h"
#include "ss.h"
#include "ht.h"
#include "msg.h"

static int dht_rput(peer_list_t *pl, char *key, char *val, int rep)
{
	char *message, ack[4] = "ERR", c = 'p';
	int connfd;
	peer_t target_peer;

	target_peer = pl->list[locate_drep_peer(key, pl, rep)];

	message = alloc_message(c, key, val);
	if (!message) {
		printf("key size should be at most 24 bytes\n"
				   "and value size at most 1000 bytes\n");
		return -1;
	}

	connfd = open_clientfd(target_peer.host, target_peer.port);
	send(connfd, message, MSG_BYTES, 0);
	recv(connfd, ack, 4, 0);

	free_message(message);
	close(connfd);

	return (!strcmp(ack, "ERR")) ? -1 : 0;
}

int dht_put(peer_list_t *pl, char *key, char *val)
{
	int ret0 = dht_rput(pl, key, val, 0),
			ret1 = dht_rput(pl, key, val, 1),
			ret2 = dht_rput(pl, key, val, 2);

	if (ret0 == 0)
		return 1;
	else if (ret1 == 0)
		return 2;
	else if (ret2 == 0)
		return 3;
	else
		return -1;
}

static int get_del(peer_list_t *pl, char *key, char *response, int dflag, int rep)
{
	char c, *message;
	int connfd;
	peer_t target_peer;

	c = (dflag) ? 'd' : 'g';

	target_peer = pl->list[locate_drep_peer(key, pl, rep)];

	message = alloc_message(c, key, "\0");
	if (!message) {
		printf("key size should be at most 24 bytes\n");
		return -1;
	}

	connfd = open_clientfd(target_peer.host, target_peer.port);
	send(connfd, message, MSG_BYTES, 0);
	recv(connfd, response, BUFSIZ, 0);

	close(connfd);
	free_message(message);

	if (response[0] == '\0' ||
			!strcmp(response, "ERR") ||
			!strcmp(response, "NOVAL"))
		return -1;

	return 0;
}


int dht_get(peer_list_t *pl, char *key, char *val)
{
	if (get_del(pl, key, val, 0, 0) == 0)
		return 1;
	else if (get_del(pl, key, val, 0, 1) == 0)
		return 2;
	else if (get_del(pl, key, val, 0, 2) == 0)
		return 3;
	else
		return -1;
}

int dht_del(peer_list_t *pl, char *key, char *val)
{
	get_del(pl, key, val, 1, 0);
	get_del(pl, key, val, 1, 1);
	get_del(pl, key, val, 1, 2);
	return 0;
}

