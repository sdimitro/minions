#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <pthread.h>

#include "mfapi.h"
#include "dht_conf.h"
#include "ss.h"
#include "ht.h"
#include "msg.h"
#include "dht_api.h"

#define UNUSED(X) (void)X

peer_list_t *pl;

/* string trim last char -
 *  * actually used for newlines */
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

static void put_cmd(char *cmd)
{
	char key[BUFSIZ], val[BUFSIZ];
	int ret;

	if (sscanf(cmd, "%*s %s %s",
				key, val) != 2) {
		printf("Incorrect number of arguments"
				   " for put command\n");
		return;
	}
	ret = dht_put(pl, key, val);

	switch(ret) {
		case 1: printf("OK\n"); break;
		case 2:
		case 3:
			printf("OK (replica %d was used)\n", ret);
			break;
		default:
			printf("error: No peer or replicas available\n");
			break;
	}
}

static void get_cmd(char *cmd)
{
	char key[BUFSIZ], val[BUFSIZ];
	int ret;

	if (sscanf(cmd, "%*s %s",
				key) != 1) {
		printf("Incorrect number of arguments"
				   " for get command\n");
		return;
	}
	memset(val, 0, BUFSIZ);
	ret = dht_get(pl, key, val);

	switch(ret) {
		case 1: printf("%s\n", val); break;
		case 2:
		case 3:
			printf("%s (replica %d was used)\n", val, ret);
			break;
		default:
			printf("NOT FOUND\n");
			break;
	}
}

static void del_cmd(char *cmd)
{
	char key[BUFSIZ], val[BUFSIZ];

	if (sscanf(cmd, "%*s %s",
				key) != 1) {
		printf("Incorrect number of arguments"
				   " for del command\n");
		return;
	}
	memset(val, 0, BUFSIZ);
	dht_del(pl, key, val);

	printf("OK?\n");
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

	if (!strcmp(token, "put")) {
		put_cmd(cmd);
	} else if (!strcmp(token, "get")) {
		get_cmd(cmd);
	} else if (!strcmp(token, "del")) {
		del_cmd(cmd);
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
		UNUSED(g);
		done = execute_cmd(input_buf);
		memset(input_buf, 0, BUFSIZ);
	}
}


int main(int c, char *v[])
{
	UNUSED(c);
	UNUSED(v);
	pl = alloc_peer_state();
	peer_repl();
	free_peer_state(pl);
	return 0;
}

