#ifndef __GEMTC_API_H
#define __GEMTC_API_H

#include <pthread.h>

void gemtc_setup(int queue_size, int workers);
void gemtc_cleanup();

void gemtc_push(int type, int threads, int id, void *params);
void gemtc_poll(int *id, void **params);

void gemtc_memcpy_host2dev(void *host, void *device, size_t size);
void gemtc_memcpy_dev2host(void *device, void *host, size_t size);

void *gemtc_malloc(unsigned int nbytes);
void gemtc_free(void *loc);


#endif /* __GEMTC_API_H */
