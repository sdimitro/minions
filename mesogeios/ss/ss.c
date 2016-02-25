#include "ss.h"

int open_clientfd(char *hostname, int port)
{
  int clientfd;
  struct hostent *hp;
  struct sockaddr_in serveraddr;

  if ((clientfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    return -1; /* check errno for cause */

  /* fill in the server's IP address and port */
  if ((hp = gethostbyname(hostname)) == NULL)
    return -2; /* check h_errno for cause */
  bzero((char *) &serveraddr, sizeof(serveraddr));
  serveraddr.sin_family = AF_INET;
  bcopy((char *) hp->h_addr_list[0],
        (char *) &serveraddr.sin_addr.s_addr, hp->h_length);
  serveraddr.sin_port = htons((unsigned short) port);

  /* establish a connection with the server */
  if (connect(clientfd, (SA *) &serveraddr, sizeof(serveraddr)) < 0)
    return -1;

  return clientfd;
}

int open_listenfd(int port)
{
  int listenfd, optval = 1;
  struct sockaddr_in serveraddr;

  /* create socket descriptor */
  if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    return -1;

  /* eliminate "address already in use" error from bind() */
  if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR,
                 (const void *) &optval, sizeof(int)) < 0)
    return -1;

  /* listenfd will be an end point for all requests to port
   * on any IP address for this host */
  bzero((char *) &serveraddr, sizeof(serveraddr));
  serveraddr.sin_family = AF_INET;
  serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
  serveraddr.sin_port = htons((unsigned short) port);
  if (bind(listenfd, (SA *) &serveraddr, sizeof(serveraddr)) < 0)
    return -1;

  /* make it a listening socket ready to accept
   * connection requests */
  if (listen(listenfd, LISTENQ) < 0)
    return -1;

  return listenfd;
}
