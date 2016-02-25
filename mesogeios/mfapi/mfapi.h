#ifndef __MFAPI_H
#define __MFAPI_H

/*
 * --------- Mike's File API (MFAPI) ---------
 *
 * An attempt to provide a more convenient interface
 * on getting file data and metadata.
 */

#include <stdlib.h>

extern long fsize(const char *filename);
extern long ftob(const char *filename, char *buf, size_t n);
extern long btof(const char *filename, char *buf, size_t n);
extern void mmap_file(const char *filename, void **buf, size_t n, int *fd);
extern void unmmap_file(void *buf, size_t n, int fd);

#endif /* __MFAPI_H */
