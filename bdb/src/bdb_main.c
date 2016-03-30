#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/*
 * unistd.h should have been enough
 * but since I use -std=c11 with
 * gcc and I'm compiling on Linux
 * I need to include getopt.h
 */
#include <getopt.h>

#include "bdb_session.h"

char *prog_name;

void
usage(void)
{
	printf("This is Brick's Debugger. Usage:\n"
	       "\n\tbdb [-v] [-p procID] [-c code]"
	       "\n\t    [prog]\n"
	       "\nReport bugs to:\n\t"
	       "https://github.com/sdimitro/minions\n");
}

struct bdb_session *
init_session(int c, char *v[])
{
	struct bdb_session *b = make_bdb_session();

	int ch = -1;
	while ((ch = getopt(c, v, "hvp:")) != -1) {
		switch (ch) {
		case 'p':
			b->pid = atoi(optarg);
			b->type = PID_ATTACH;
			break;
		case 'v':
			b->verbose = 1;
			break;
		case 'h':
		case '?':
		default:
			usage();
			break;
		}
	}

	if (b->verbose)
		printf("PID: %d\n", b->pid);

	return b;
}

int
main(int c, char *v[])
{
	prog_name = v[0];
	struct bdb_session *session = init_session(c, v);
	printf("Where did you get your debugger, at the toilet store?\n");
	clean_bdb_session(session);
}

