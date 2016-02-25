#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

#include "sbuf.h"
#include "rio.h"
#include "ss.h"
#include "mfapi.h"

#define MAXFILENAME 100

sbuf_t sbuf; /* shared buffer of connected descriptors */

void respond_request(int fd);
void *thread(void *vargp);

void peerd(int nthreads, int qsize, int port)
{
        int listenfd, connfd, i;
        struct sockaddr_in clientaddr;
        socklen_t clientlen = sizeof(struct sockaddr_in);
        pthread_t tid;

        sbuf_init(&sbuf, qsize);
        listenfd = open_listenfd(port);

        for (i = 0; i < nthreads; i++)
                pthread_create(&tid, NULL, thread, NULL);

        while (1) {
                connfd = accept(listenfd, (SA *) &clientaddr,
                                &clientlen);
                int *cfd_ptr = (int *) malloc(sizeof(int));
                *cfd_ptr = connfd;
                sbuf_insert(&sbuf, cfd_ptr);
        }
}

void respond_request(int cfd)
{
        char filepath[MAXFILENAME], *mbuf;
        int mfd;
        long size = 0;

        recv(cfd, filepath, MAXFILENAME, 0);
        size = fsize(filepath);
        send(cfd, &size, sizeof(long), 0);
        if (size <= 0) { return; }
        mmap_file(filepath, (void **) &mbuf, size, &mfd);
        rio_writen(cfd, mbuf, size);
        unmmap_file(mbuf, size, mfd);
}

void *thread(void *vargp)
{
        (void)vargp;
        pthread_detach(pthread_self());
        while(1) {
                int *cfd_ptr = (int *) sbuf_remove(&sbuf);
                respond_request(*cfd_ptr);
                close(*cfd_ptr);
                free(cfd_ptr);
        }
}
