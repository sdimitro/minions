#include "mfapi.h"
#include "dht_api.h"
#include "dht_conf.h"

#include <stdio.h>
#include <string.h>

void file_index(peer_list_t *p, char *f, char *addr)
{
	int ret;

	if (fsize(f) == -1) {
		printf("%s can't be found.\n", f);
		return;
	}

	ret = dht_put(p, f, addr);

	switch(ret) {
	case 1:
		printf("OK\n");
		break;
	case 2: case 3:
		printf("OK (replica %d was used)\n", ret);
		break;
	default:
		printf("error: the peer and its replicas are not available\n");
		break;
	}
}

void locate(peer_list_t *p, char *f, char *loc)
{
	int ret;

	memset(loc, 0, BUFSIZ);
	ret = dht_get(p, f, loc);

	switch(ret) {
	case 1:
		printf("%s - OK\n", loc);
		break;
	case 2: case 3:
		printf("%s - OK (replica %d was used)\n", loc, ret);
		break;
	default:
		printf("NOT FOUND\n");
		break;
	}
}

/*

void get(char *cmd)
{
        char filepath[BUFSIZ],
             peer[BUFSIZ],
             *buf;
        int clientfd, port;
        long fsz;

        if(sscanf(cmd, "%*s %s %s %d",
                filepath, peer, &port) != 3) {
                printf("Incorrect number of arguments"
                       " for get command\n");
                return;
        }

        printf("Attempting to get %s from "
               "%s at port %d ...\n",
               filepath, peer, port);
        clientfd = open_clientfd(peer, port);
        send(clientfd, filepath, strlen(filepath) + 1, 0);
        recv(clientfd, &fsz, sizeof(long), 0);

        buf = (char *) malloc(fsz);
        rio_readn(clientfd, buf, fsz);
        btof(filepath, buf, fsz);

        close(clientfd);
        free(buf);

        printf("DONE\n");
}
*/

