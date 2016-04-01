#ifndef __BDB_UTILS_H
#define __BDB_UTILS_H

#include <sys/ptrace.h>
#include <sys/types.h>

#include <stdlib.h>

void unix_error(char *msg);
void *xmalloc(size_t size);
void xfree(void *ptr);
void *xcalloc(size_t nmemb, size_t size);

long xptrace(enum __ptrace_request request, pid_t pid,
		void *addr, void *data);

#endif /* __BDB_UTILS_H */

