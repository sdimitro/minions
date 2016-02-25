#ifndef __SS_H
#define __SS_H

/*
 * --------- Simple(r) Sockets (SS) ---------
 * (taken from CS:APP2 from Bryant & O'Hallaron)
 *
 * - Why SS?
 *
 * It is just more convenient for example to wrap socket()
 * and connect() into a helper function than to just use
 * them directly within our program.
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <strings.h>

#define LISTENQ 1024

/* The following type is  used to cast sockaddr_in structs
 * to generic sockaddr structs */
typedef struct sockaddr SA;

/* Wrapper functions */
extern int open_clientfd(char *hostname, int port);
extern int open_listenfd(int port);

#endif /* __SS_H */
