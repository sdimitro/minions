#ifndef __BDB_UTILS_H
#define __BDB_UTILS_H

#include <stdlib.h>

void unix_error(char *msg);
void *xmalloc(size_t size);
void xfree(void *ptr);
void *xcalloc(size_t nmemb, size_t size);

#endif /* __BDB_UTILS_H */

