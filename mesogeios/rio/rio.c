#include "rio.h"

ssize_t rio_readn(int fd, void *usrbuf, size_t n)
{
  size_t nleft = n;
  ssize_t nread;
  char *bufp = usrbuf;

  while (nleft > 0) {
    if ((nread = read(fd, bufp, nleft)) < 0) {
      /* If you get interrupted by a signal handler
       * we call read() again. Else we return -1,
       * at that point errno would be set by read() */
      if (errno == EINTR)
        nread = 0;
      else
        return -1;
    }
    else if (nread == 0)
      break;          /* EOF */
    nleft -= nread;
    bufp += nread;
  }

  return (n - nleft); /* return >= 0 */
}

ssize_t rio_writen(int fd, void *usrbuf, size_t n)
{
  size_t nleft = n;
  ssize_t nwritten;
  char *bufp = usrbuf;

  while (nleft > 0) {
    if ((nwritten = write(fd, bufp, nleft)) < 0) {
      /* If you get interrupted by a signal handler
       * we call read() again. Else we return -1,
       * at that point errno would be set by read() */
      if (errno == EINTR)
        nwritten = 0;
      else
        return -1;
    }

    nleft -= nwritten;
    bufp += nwritten;
  }

  return n;
}

void rio_readinitb(rio_t *rp, int fd)
{
  rp->rio_fd = fd;
  rp->rio_cnt = 0;
  rp->rio_bufptr = rp->rio_buf;
}

static ssize_t rio_read(rio_t *rp, char *usrbuf, size_t n)
{
  int cnt;

  /* refill buf if it is empty */
  while (rp->rio_cnt <= 0) {
    rp->rio_cnt =  read(rp->rio_fd, rp->rio_buf,
                       sizeof(rp->rio_buf));

    /* If read() returned an error and that wasn't caused
     * by a sig handler interruption return error (errno
     * is set by read() ). If we hit an EOF return 0.
     * Else everything went well and we reset the buffer ptr.*/
    if (rp->rio_cnt < 0) {
      if (errno != EINTR)
        return -1;
    }
    else if (rp->rio_cnt == 0)
      return 0;
    else
      rp->rio_bufptr = rp->rio_buf;
  }

  /* copy min(n, rp->rio_cnt) bytes from the
   * internal buffer to the user buffer.*/
  cnt = (rp->rio_cnt < n) ? rp->rio_cnt : n;
  memcpy(usrbuf, rp->rio_buf, cnt);
  rp->rio_bufptr += cnt;
  rp->rio_cnt -= cnt;

  return cnt;
}

ssize_t rio_readlineb(rio_t *rp, void *userbuf, size_t maxlen)
{
  int n, rc;
  char c, *bufp = userbuf;

  for (n = 1; n < maxlen; n++) {
    if ((rc = rio_read(rp, &c, 1)) == 1) {
      *bufp++ = c;
      if (c == '\n')
        break;
    } else if (rc == 0) {
      if (n == 1)
        return 0; /* EOF, no data was read */
      else
        break;    /* EOF, some data was read */
    } else
      return -1;
  }

  *bufp = 0;
  return n;
}

ssize_t rio_readnb(rio_t *rp, void *userbuf, size_t n)
{
  size_t nleft = n;
  ssize_t nread;
  char *bufp = userbuf;

  while (nleft > 0) {
    if ((nread = rio_read(rp, bufp, nleft)) < 0) {
      /* If you get interrupted by a signal handler
       * we call read() again. Else we return -1,
       * at that point errno would be set by read() */
      if (errno == EINTR)
        nread = 0;
      else
        return -1;
    }
    else if (nread == 0)
      break;          /* EOF */
    nleft -= nread;
    bufp += nread;
  }

  return (n - nleft); /* return >= 0 */
}
