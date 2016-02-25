#define _XOPEN_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>

#include <sys/types.h>
#include <signal.h>

#include "mfapi.h"
#include "dht_conf.h"
#include "ss.h"
#include "ht.h"
#include "msg.h"
#include "dht_api.h"
#include "peer_func.h"
#include "rio.h"

peer_list_t *pl;
char *iaddr_port;

void peerd(int nthreads, int qsize, int port);

/* string trim last char -
 * actually used for newlines */
static void strtlc(char str[])
{
	int len = strlen(str);
	str[len - 1] = '\0';
}

static void print_help()
{
	printf("Available commands -\n");
	printf("  put  <key>  <value>\n");
	printf("  get  <key>\n");
	printf("  del  <key>\n");
}

static void index_cmd(char *cmd)
{
	char f[BUFSIZ];

	if (sscanf(cmd, "%*s %s", f) != 1) {
		printf("Incorrect number of arguments"
				   " for put command\n");
		return;
	}

	file_index(pl, f, iaddr_port);
}

static void find_cmd(char *cmd)
{
	char f[BUFSIZ], loc[BUFSIZ];

	if (sscanf(cmd, "%*s %s", f) != 1) {
		printf("Incorrect number of arguments"
				   " for get command\n");
		return;
	}
	locate(pl, f, loc);
}

void get_cmd(char *cmd)
{
	char filepath[BUFSIZ],
	     peer[BUFSIZ],
	     *buf;
	int clientfd, port;
	long fsz;
	
	if (sscanf(cmd, "%*s %s %s %d",
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

static int execute_cmd(char cmd[])
{
	char token[BUFSIZ];
	size_t size;
	char *p;

	if (!strcmp(cmd, "quit\n")
			|| !strcmp(cmd, "q\n")
			|| !strcmp(cmd, "")) {
		return 1;
	}
	strtlc(cmd);

	p = strchr(cmd, ' ');
	if (!p) { print_help(); return 0; }

	size = p - cmd;
	strncpy(token, cmd, size);
	token[size] = '\0';

	if (!strcmp(token, "index")) {
		index_cmd(cmd);
	} else if (!strcmp(token, "find")) {
		find_cmd(cmd);
	} else if (!strcmp(token, "get")) {
		get_cmd(cmd);
	} else {
		print_help();
	}

	return 0;
}

static void peer_repl(void)
{
	char input_buf[BUFSIZ];

	char *repl = ">";
	int done = 0;

	while (!done) {
		printf("%s ", repl);
		void *g = fgets(input_buf, BUFSIZ, stdin);
		(void)g;
		done = execute_cmd(input_buf);
		memset(input_buf, 0, BUFSIZ);
	}
}

int parse_port(char *s)
{
	return atoi((index(s, ':') + 1));
}

int main(int c, char *v[])
{
	int port;

	if (c != 4) {
		printf("usage: %s <ip:port> <# threads> <queue size>\n", v[0]);
		return -1;
	}

	iaddr_port = v[1];
	port = parse_port(v[1]);

	pid_t pid = fork();
	if (pid == 0) { peerd(atoi(v[2]), atoi(v[3]), port); }

	pl = alloc_peer_state();
	peer_repl();
	free_peer_state(pl);

	kill(pid, SIGKILL); /* ruthless! */
	return 0;
}

