#ifndef __RIO_H
#define __RIO_H

/*
 * --------- Robust I/O (RIO) ---------
 * (taken from CS:APP2 from Bryant & O'Hallaron)
 *
 * - Why RIO?
 *
 * Applications such as network programs that use plain
 * read() and write() are subject to 'short counts'. Short
 * counts happen when read() and write() transfer fewer
 * bytes than the application requests. Short counts occur
 * when:
 *
 * 1] we encounter EOF on reads.
 * 2] we read text lines from a terminal.
 * 3] we read and write to/from network sockets or UNIX
 *    pipes.
 *
 * Short counts don't happen  when reading to disk (except
 * on EOF) or writing to disk. However, to build robust
 * network applications, we must deal with short counts and
 * repeatedly call read() and write() until all requested
 * bytes have been transferred.
 *
 * - What does RIO provide?
 *
 * 1] Unbuffered I/O functions.
 *    They transfer data directily between memory and file
 *    without any application level buffering. These are
 *    generally useful for reading and writing binary data
 *    to and from networks.
 *
 * 2] Buffered input functions.
 *    They allow you to efficiently read text lines and
 *    binary data whose contents are cached in an
 *    application-level buffer, similar to standard I/O
 *    functions. They are thread-safe and can be interleaved
 *    arbitrarily on the same descriptor.
 *
 * - Is there anything we should be careful about?
 *
 * Calls to the buffered functions should not be interleaved
 * with to calls with calls to the unbuffered rio_readn().
 */

#include <unistd.h>
#include <errno.h>
#include <string.h>

#define RIO_BUFSIZE 8192

typedef struct {
  int rio_fd;                /* descriptor for this internal buf */
  int rio_cnt;               /* unread bytes in internal buf  */
  char *rio_bufptr;          /* next unread byte in internal buf  */
  char rio_buf[RIO_BUFSIZE]; /* internal buffer */
} rio_t;

/* RIO unbuffered I/O functions */
ssize_t rio_readn(int fd, void *usrbuf, size_t n);
ssize_t rio_writen(int fd, void *usrbuf, size_t n);

/* RIO buffered input functions */
void rio_readinitb(rio_t *rp, int fd);
ssize_t rio_readlineb(rio_t *rp, void *userbuf, size_t maxlen);
ssize_t rio_readnb(rio_t *rp, void *userbuf, size_t n);

#endif /* __RIO_H */
