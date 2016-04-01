#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <errno.h>

#include "bdb_utils.h"

void
unix_error(char *msg)
{
	fprintf(stderr,
		"%s error: %s\n",
		msg, strerror(errno));
	exit(EXIT_FAILURE);
}

void *
xmalloc(size_t size)
{
	void *p = malloc(size);
	if (!p) unix_error("xmalloc");
	return p;
}

void
xfree(void *ptr)
{
	if (!ptr) return;
	free(ptr);
}

void *
xcalloc(size_t nmemb, size_t size)
{
	void *p = calloc(nmemb, size);
	if (!p) unix_error("xcalloc");
	return p;
}

long
xptrace(enum __ptrace_request request, pid_t pid, void *addr, void *data)
{
	long rv = ptrace(request, pid, addr, data);
	if (rv == -1) unix_error("xptrace");
	return rv;
}

