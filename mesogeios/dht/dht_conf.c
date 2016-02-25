#include "dht_conf.h"
#include "ht.h"
#include "mfapi.h"

#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <strings.h>

peer_list_t *alloc_peer_list(char *buf)
{
	int i, plist_sz, port, host_len;
	char host[BUFSIZ];

	peer_list_t *pl = (peer_list_t *) malloc(sizeof(peer_list_t));

	sscanf(buf, "%d", &plist_sz);
	pl->size = plist_sz;
	pl->list = (peer_t *) malloc(sizeof(peer_t) * plist_sz);

	buf = index(buf, '\n') + 1;
	for (i = 0; i < plist_sz; i++) {
		sscanf(buf, "%s %d", host, &port);

		host_len = strlen(host);

		(pl->list[i]).port = port;
		(pl->list[i]).host = (char *) malloc(sizeof(char) * host_len);
		strncpy((pl->list[i]).host, host, host_len);

		buf = index(buf, '\n') + 1;
	}

	return pl;
}

peer_list_t *alloc_peer_state(void)
{
	peer_list_t *pl;
	char *buf;
	int mfd;
	long size;

	size = fsize(CONFIG_FILENAME);
	mmap_file(CONFIG_FILENAME, (void **) &buf, size, &mfd);
	pl = alloc_peer_list(buf);
	unmmap_file(buf, size, mfd);
	return pl;
}

void free_peer_state(peer_list_t *pl)
{
	int i;
	for (i = 0; i < pl->size; i++)
		free((pl->list[i]).host);
	free(pl->list);
	free(pl);
}

int locate_dpeer(char *key, peer_list_t *pl)
{
	return hash_function(pl->size, key);
}

int locate_drep_peer(char *key, peer_list_t *pl, int repn)
{
	return hash_function_rep(repn, pl->size, key);
}

