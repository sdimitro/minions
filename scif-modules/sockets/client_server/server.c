#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <errno.h>
#include <pthread.h>

#include "gemtc_api.h"

/* TODO: May have to change that */
#define MAXEVENTS 64

#define NOCONTENT 0

static int make_socket_non_blocking(int sfd)
{
	int flags, s;

	flags = fcntl(sfd, F_GETFL, 0);

	if (flags == -1) {
		perror ("fcntl");
		return -1;
	}

	flags |= O_NONBLOCK;
	s = fcntl (sfd, F_SETFL, flags);

	if (s == -1) {
		perror ("fcntl");
		return -1;
	}

	return 0;
}

static int create_and_bind(char *port)
{
	struct addrinfo hints;
	struct addrinfo *result, *rp;
	int s, sfd;

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;     /* Return IPv4 and IPv6 choices */
	hints.ai_socktype = SOCK_STREAM; /* TCP socket */
	hints.ai_flags = AI_PASSIVE;     /* All interfaces */

	s = getaddrinfo(NULL, port, &hints, &result);
	if (s != 0) {
		fprintf (stderr, "getaddrinfo: %s\n", gai_strerror(s));
		return -1;
	}

	for (rp = result; rp != NULL; rp = rp->ai_next) {
		sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if (sfd == -1)
			continue;

		s = bind(sfd, rp->ai_addr, rp->ai_addrlen);
		if (s == 0) {
			/* Managed to bind successfully! */
			break;
		}

		close(sfd);
	}

	if (rp == NULL) {
		fprintf (stderr, "Could not bind\n");
		return -1;
	}

	freeaddrinfo (result);

	return sfd;
}

static void enqueue_from(int fd, size_t message_size, ssize_t bytes_read)
{
	void *whole_message, *params, *task_info;
	int type, threads, id, count;

	whole_message = malloc(message_size + sizeof(size_t));
	*((size_t *) whole_message) = message_size;
	task_info = (void *) (((char *) whole_message) + sizeof(size_t));
	

	count = read(fd, task_info, message_size);
	if (count == -1 && errno != EAGAIN) {
		/* If errno == EAGAIN, that means we have read all data. */
		perror("read");
		free(whole_message);
		return;

	} else if (count == 0) {
		/* End of file. The remote has closed the connection. */
		free(whole_message);
		return;
	}
	bytes_read += count;
	printf("# Received: %zd bytes - Content: %zu bytes\n", bytes_read, message_size);

	/*
	 * Extract task_info:
	 * -----------------------------------------------------------
	 * | type (int) | threads (int) | id (int) | params (void *) |
	 * -----------------------------------------------------------
	 */
	type = *((int *) task_info);
	threads = *((int *) task_info + 1);
	id = *((int *) task_info + 2);
	params = (void *) (((char *) task_info) + sizeof(int) * 3);

	printf("# Push job - ID = %d - Type = %d\n", id, type);

	gemtc_push(type, threads, id, params);
}

static void dequeue_to(int fd, ssize_t bytes_read)
{
	ssize_t bytes_sent;
	size_t reply_size;
	void *reply, *params;
	int id;

	printf("# Received: %zd bytes - Content: %d bytes\n", bytes_read, NOCONTENT);

	gemtc_poll(&id, &params);
	printf("# Dequeued job - ID = %d\n", id);

	/* Recover the original message */
	reply = (void *) (((char *) params) - 3 * sizeof(int) - sizeof(size_t));
	reply_size = *((size_t *) reply) + sizeof(size_t);

	/*
	 * WARNING: Assumptions about state.
	 * The idea is that gemtc_poll will mutate the state of the
	 * received message to portray the state of the result.
	 * Therefore params have the result now.
	 */
	bytes_sent = write(fd, reply, reply_size);
	if (bytes_sent == -1) {
		perror("write");
		goto cleanup_message;

	} else if (bytes_sent == 0) {
		/* End of file. The remote has closed the connection. */
		goto cleanup_message;
	}

	/* Log to standard output */
	printf("# Sent: %zd bytes - Content size: %zu bytes\n",
		bytes_sent, reply_size);

	cleanup_message:
	free(reply);
}

void *thread(void *arg)
{
	int connfd = *((int *) arg);
	ssize_t bytes_read = *((ssize_t *) (((char *) arg) + sizeof(int)));
	size_t message_size = *((size_t *) (((char *) arg) +
		sizeof(int) + sizeof(ssize_t)));
	pthread_detach(pthread_self());
	free(arg);

	if (message_size == 1)
		dequeue_to(connfd, bytes_read);
	else
		enqueue_from(connfd, message_size, bytes_read);

	printf("Closed connection on descriptor %d\n", connfd);

	/*
	 * Closing the descriptor will make epoll remove it
	 * from the set of descriptors which are monitored.
	 */
	close(connfd);

	return NULL;
}

int main(int argc, char *argv[])
{
	int sfd, s;
	int efd;
	int workers, queue_size;
	struct epoll_event event;
	struct epoll_event *events;

	if (argc != 4) {
		fprintf(stderr, "Usage: %s [port] [workers] [queue size]\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	workers = atoi(argv[2]);
	queue_size = atoi(argv[3]);

	/* setup framework */
	gemtc_setup(queue_size, workers);

	sfd = create_and_bind(argv[1]);
	if (sfd == -1)
		abort();

	s = make_socket_non_blocking(sfd);
	if (s == -1)
		abort();

  s = listen(sfd, SOMAXCONN);
	if (s == -1) {
		perror("listen");
		abort();
	}

	efd = epoll_create1(0);
	if (efd == -1) {
		perror("epoll_create");
		abort();
	}

	event.data.fd = sfd;
	event.events = EPOLLIN | EPOLLET;
	s = epoll_ctl(efd, EPOLL_CTL_ADD, sfd, &event);
	if (s == -1) {
		perror ("epoll_ctl");
		abort ();
	}

	/* Buffer where events are returned */
	events = calloc(MAXEVENTS, sizeof(event));

	/* The event loop */
	while (1) {
		int n, i;

		n = epoll_wait(efd, events, MAXEVENTS, -1);

		for (i = 0; i < n; i++) {
			if ((events[i].events & EPOLLERR) ||
					(events[i].events & EPOLLHUP) ||
					(!(events[i].events & EPOLLIN))) {

				/*
				 * An error has occured on this fd, or the socket is not
				 * ready for reading (why were we notified then?)
				 */
				fprintf(stderr, "epoll error\n");
				close(events[i].data.fd);
				continue;

			} else if (sfd == events[i].data.fd) {
				/*
				 * We have a notification on the listening socket, which
				 * means one or more incoming connections.
				 */
				while (1) {
					struct sockaddr in_addr;
					socklen_t in_len;
					int infd;
					char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];

					in_len = sizeof(in_addr);
					infd = accept(sfd, &in_addr, &in_len);
					if (infd == -1) {
						if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
							/* We have processed all incoming connections. */
							break;
						} else {
							perror("accept");
							break;
						}
					}

					s = getnameinfo(&in_addr, in_len, hbuf, sizeof(hbuf),
						sbuf, sizeof(sbuf), NI_NUMERICHOST | NI_NUMERICSERV);

					if (s == 0) {
						printf("Accepted connection on descriptor %d "
							"(host=%s, port=%s)\n", infd, hbuf, sbuf);
					}

					/*
					 * Make the incoming socket non-blocking and add it to the
					 * list of fds to monitor.
					 */
					s = make_socket_non_blocking(infd);
					if (s == -1)
						abort();

					event.data.fd = infd;
					event.events = EPOLLIN | EPOLLET;
					s = epoll_ctl (efd, EPOLL_CTL_ADD, infd, &event);
					if (s == -1) {
						perror("epoll_ctl");
						abort();
					}
				}
				continue;

			} else {
				/*
				 * We have data on the fd waiting to be read. Read and
				 * display it. We must read whatever data is available
				 * completely, as we are running in edge-triggered mode
				 * and won't get a notification again for the same
				 * data.
				 */
				int done = 0;
				ssize_t bytes_read;
				size_t message_size;
				pthread_t tid;
				void *thread_args;

				bytes_read = read(events[i].data.fd, &message_size, sizeof(size_t));
				if (bytes_read == -1 && errno != EAGAIN) {
					/* If errno == EAGAIN, that means we have read all data. */
					perror("read");
					close(events[i].data.fd);
					done = 1;

				} else if (bytes_read == 0) {
					/* End of file. The remote has closed the connection. */
					close(events[i].data.fd);
					done = 1;
				}

				if (!done) {
					if (message_size == 0) {
						/* This is the shutdown command! */
						goto cleanup_shutdown;

					} else {
						/* REFACTOR: So ugly! */
						thread_args = malloc(sizeof(int) +
							sizeof(ssize_t) + sizeof(size_t));
						*((int *) thread_args) = events[i].data.fd;
						*((ssize_t *) (((char *) thread_args) +
							sizeof(int))) = bytes_read;
						*((size_t *) (((char *) thread_args) +
							sizeof(int) + sizeof(ssize_t))) = message_size;
						pthread_create(&tid, NULL, thread, thread_args);
					}
				}
			}
		}
	}

	cleanup_shutdown:
	printf("Server is shutting down!\n");
	free(events);
	close(sfd);
	gemtc_cleanup();

	return EXIT_SUCCESS;
}
